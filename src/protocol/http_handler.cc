#include "protocol/http_handler.hh"

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>

#include "application/file_service.hh"
#include "application/service.hh"
#include "boost/asio/completion_condition.hpp"
#include "boost/asio/error.hpp"
#include "boost/asio/read.hpp"
#include "boost/system/detail/error_code.hpp"
#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/parser.hh"
#include "protocol/response.hh"
#include "protocol/uri_decoder.hh"

// TODO(thebao): Reduce cognitive complexity of handleRequest function
void HTTPHandler::handleRequest() {
  boost::asio::async_read_until(*socket_, buffer_, END_OF_REQUEST,
                                [this](const boost::system::error_code& ecd, std::size_t length) {
                                  handleOneRequest(ecd, length);
                                });
}

void HTTPHandler::handleOneRequest(const boost::system::error_code& ecd, std::size_t length) {
  using boost::asio::error::connection_reset;
  using boost::asio::error::eof;
  using boost::system::error_code;
  log("Received {} bytes", length);

  if (!ecd) {
    processRequest();
  } else if (ecd == eof || ecd == connection_reset) {
    // 连接关闭，不处理请求
    log("Connection closed.");
    socket_->close();
  } else {
    error("Read error: {}", ecd.message());
  }
}

auto HTTPHandler::getRequest() -> std::string {
  auto        begin = boost::asio::buffers_begin(buffer_.data());
  auto        end   = boost::asio::buffers_end(buffer_.data());
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

auto HTTPHandler::getBody() -> void {
  auto content_length    = request_ctx_.getHeader("Content-Length");
  auto transfer_encoding = request_ctx_.getHeader("Transfer-Encoding");

  if (content_length.empty() && transfer_encoding == "chunked") {
    getBodyWithChunked();
  } else if (!content_length.empty()) {
    std::size_t length = std::stoul(content_length);
    log("Expected body length {}", length);
    getBodyWithLength(length);
  } else {
    processBodyComplete();
  }
}

auto HTTPHandler::getBodyWithLength(std::size_t length) -> void {
  using boost::asio::async_read;
  using boost::asio::buffer;
  using boost::asio::buffers_begin;
  using boost::asio::transfer_at_least;
  using boost::system::error_code;

  std::function<void(const error_code& ecd, size_t length)> handler =
      [this, length, &handler](const error_code& ecd, std::size_t bytes_transferred) mutable {
        if (!ecd) {
          std::string data(
              buffers_begin(buffer_.data()),
              buffers_begin(buffer_.data()) + static_cast<std::ptrdiff_t>(bytes_transferred));
          buffer_.consume(bytes_transferred);
          request_ctx_.addBody(data);

          size_t received = request_ctx_.getBody().size();
          if (received < length) {
            // 继续读取剩余部分
            async_read(*socket_, buffer_, transfer_at_least(1), handler);
          } else {
            processBodyComplete();
          }
        } else {
          if (ecd == boost::asio::error::eof || ecd == boost::asio::error::connection_reset) {
            log("Connection closed.");
            socket_->close();
          } else {
            error("Read error: {}", ecd.message());
          }
        }
      };

  // 先检查缓冲区有无需要的数据, 再异步读取
  if (buffer_.size() >= length) {
    std::string data(buffers_begin(buffer_.data()),
                     buffers_begin(buffer_.data()) + static_cast<std::ptrdiff_t>(length));
    buffer_.consume(length);
    request_ctx_.addBody(data);
    processBodyComplete();
    return;
  }

  async_read(*socket_, buffer_, transfer_at_least(1), handler);
}

auto HTTPHandler::getBodyWithChunked() -> void {
  // TODO(thebao): 处理chunked transfer encoding
}

// TODO(thebao): 处理异步post请求体
void HTTPHandler::processRequest() {
  // ----------------- Request -----------------
  std::string request = getRequest();
  request_ctx_        = Parser::parse(request);
  // ----------------- Request -----------------
  if (request_ctx_.getMethod() == "POST") {
    getBody();
  } else {
    processBodyComplete();
  }

  // ----------------- Service -----------------
  // response_ctx_   = router_.forward(request_ctx_);
  // bool keep_alive = true;
  // keep_alive                 = request_ctx_.getHeader("Connection") == "keep-alive";
  // ----------------- Service -----------------

  // ----------------- Response -----------------
  // HTTPResponse builder(response_ctx_);
  // sendResponse(builder.buildWithContext(), keep_alive);
  // ----------------- Response -----------------
}

void HTTPHandler::processBodyComplete() {
  // ----------------- Service -----------------
  response_ctx_   = router_.forward(request_ctx_);
  bool keep_alive = true;
  keep_alive      = request_ctx_.getHeader("Connection") == "keep-alive";
  // ----------------- Service -----------------

  // ----------------- Response -----------------
  HTTPResponse builder(response_ctx_);
  sendResponse(builder.buildWithContext(), keep_alive);
  // ----------------- Response -----------------
}

void HTTPHandler::sendResponse(const std::string& response, bool keep_alive) {
  using boost::asio::async_write;
  using boost::asio::buffer;
  using boost::system::error_code;

  auto self = shared_from_this();
  async_write(*socket_, buffer(response),
              [this, self, keep_alive](const error_code& ecd, std::size_t) {
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