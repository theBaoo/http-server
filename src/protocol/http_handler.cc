#include "protocol/http_handler.hh"

#include <cstddef>
#include <iostream>
#include <string>

#include "application/file_service.hh"
#include "application/service.hh"
#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/parser.hh"
#include "protocol/response.hh"
#include "protocol/uri_decoder.hh"

// TODO(thebao): Reduce cognitive complexity of handleRequest function
void HTTPHandler::handleRequest() {
  asio::async_read_until(
      *socket_, buffer_, END_OF_REQUEST,
      [this](const asio::error_code& ecd, std::size_t length) { handleOneRequest(ecd, length); });
}

auto HTTPHandler::getRequest() -> std::string {
  auto        begin = asio::buffers_begin(buffer_.data());
  auto        end   = asio::buffers_end(buffer_.data());
  std::string data(begin, end);

  auto pos = data.find("\r\n\r\n");
  if (pos == std::string::npos) {
    return ""; // 未找到分隔符，返回空字符串或抛出异常，取决于需求
  }

  std::string request = data.substr(0, pos + 4);
  buffer_.consume(pos + 4);

  log("Request: {}", request);
  return request;
}

void HTTPHandler::processRequest() {
  // ----------------- Request -----------------
  std::string request = getRequest();

  auto [method, path, version] = Parser::parseRequest(request);
  auto replaced                = URIDecoder::replacePercent(path);
  auto uri                     = URIDecoder::decode(replaced);

  std::map<std::string, std::string> headers;
  while (!request.empty()) {
    auto [field, value] = Parser::parseHeader(request);
    if (!field.empty() && !value.empty()) {
      headers[field] = value;
    } else if (request.find_first_not_of("\r\n") == std::string::npos) {
      // 读取到空行，说明请求头部解析完毕
      break;
    }
  }
  // ----------------- Request -----------------

  // ----------------- Service -----------------
  RequestContext  request_context{};
  ResponseContext response = (*router_[uri])(request_context);

  StatusCode  status_code    = response.getStatusCode();
  std::string status_message = response.getStatusMessage();
  std::string content        = response.getBody().value_or("");
  if (content.empty()) {
    status_code    = StatusCode::NOT_FOUND;
    status_message = "Not Found";
  }

  // TODO(thebao): resolve body
  if (method == "POST") {
    std::string body;
    size_t length = headers["Content-Length"].empty() ? 0 : std::stoi(headers["Content-Length"]);
    if (length > 0) {
      body.resize(length);
      auto begin = asio::buffers_begin(buffer_.data());
      body.assign(begin, begin + static_cast<std::ptrdiff_t>(length));
      buffer_.consume(length);
    }

    auto params = Parser::parseBody(body, ContentType::FORM);
    for (const auto& [field, value] : params) {
      log("Field: {}, Value: {}", field, value);
    }
  }

  bool keep_alive = false;
  if (headers.find("Connection") != headers.end()) {
    keep_alive = headers["Connection"] == "keep-alive";
  }
  // ----------------- Service -----------------

  // ----------------- Response -----------------
  HTTPResponse builder(method);
  builder.setStatus(status_code).setBody(content).addHeader("Content-Type", "text/html");

  sendResponse(builder.build(), keep_alive);
  // ----------------- Response -----------------
}

void HTTPHandler::sendResponse(const std::string& response, bool keep_alive) {
  auto self = shared_from_this();
  asio::async_write(*socket_, asio::buffer(response),
                    [this, self, keep_alive](const asio::error_code& ecd, std::size_t) {
                      void(this);
                      if (ecd) {
                        error("Write error: {}", ecd.message());
                      } else {
                        log("Response sent.");
                        if (keep_alive) {
                          log("Connection keep-alive.");
                          handleRequest();
                        } else {
                          log("Connection closed.");
                          socket_->close();
                        }
                      }
                    });
}

void HTTPHandler::handleOneRequest(const asio::error_code& ecd, std::size_t length) {
  log("Received {} bytes", length);

  if (!ecd) {
    processRequest();
  } else if (ecd == asio::error::eof || ecd == asio::error::connection_reset) {
    // 连接关闭，不处理请求
    log("Connection closed.");
    socket_->close();
  } else {
    error("Read error: {}", ecd.message());
  }
}