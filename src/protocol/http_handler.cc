#include "protocol/http_handler.hh"

#include <iostream>
#include <string>

#include "application/file_service.hh"
#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/parser.hh"
#include "protocol/response.hh"
#include "protocol/uri_decoder.hh"

void HTTPHandler::handleRequest() {
  auto self = shared_from_this();
  asio::async_read_until(
      *socket_, buffer_, END_OF_REQUEST,
      [this, self](const asio::error_code& ecd, std::size_t length) {
        Logger::getLogger("http handler").info("Request received with length {}", length);
        if (!ecd) {
          // 从 buffer_ 中提取请求数据
          std::istream request_stream(&buffer_);
          std::string  request((std::istreambuf_iterator<char>(request_stream)),
                               std::istreambuf_iterator<char>());

          auto [method, path, version] = Parser::parseRequest(request);
          auto uri                     = URIDecoder::decode(path);

          StatusCode  status_code    = StatusCode::OK;
          std::string status_message = "OK";
          std::string content        = FileService::getInstance().getFileContent(uri);
          if (content.empty()) {
            status_code    = StatusCode::NOT_FOUND;
            status_message = "Not Found";
          }

          ResponseBuilder builder;
          builder.setStatusCode(status_code);
          builder.setStatusMessage(status_message);
          builder.setBody(content);
          builder.addHeader("Server", "SimpleHTTPServer");
          builder.addHeader("Content-Type", "text/html");
          std::string response = builder.build();

          asio::async_write(*socket_, asio::buffer(response),
                            [this, self](const asio::error_code& ecd, std::size_t) {
                              void(this);
                              if (ecd) {
                                Logger::getLogger("http handler").error("Write error: {}", ecd.message());
                              } else {
                                Logger::getLogger("http handler").info("Response sent.");
                              }
                            });
        } else if (ecd == asio::error::eof || ecd == asio::error::connection_reset) {
          // 连接关闭，不处理请求
          Logger::getLogger("http handler").info("Connection closed.");
        } else {
          Logger::getLogger("http handler").error("Read error: {}", ecd.message());
        }
      });
}