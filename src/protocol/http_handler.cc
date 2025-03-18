#include "protocol/http_handler.hh"

#include <iostream>
#include <string>
#include "fmt/core.h"

#include "protocol/parser.hh"
#include "protocol/response.hh"
#include "protocol/uri_decoder.hh"
#include "application/file_service.hh"
#include "common/constants.hh"

void HTTPHandler::handleRequest() {
  auto self = shared_from_this();
  asio::async_read_until(
    *socket_,
    buffer_,
    END_OF_REQUEST,
    [this, self](const asio::error_code& ec, std::size_t length) {
      fmt::println("{}", length);
      if (!ec) {
        // 从 buffer_ 中提取请求数据
        std::istream request_stream(&buffer_);
        std::string  request((std::istreambuf_iterator<char>(request_stream)),
                            std::istreambuf_iterator<char>());
        fmt::print("Received request: {}\n", request);

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
        builder.addHeader("Content-Type", "text/html");
        std::string response = builder.build();
        
        fmt::println("{}", response);

        asio::async_write(*socket_, asio::buffer(response),
                          [this, self](const asio::error_code& ec, std::size_t) {
                            void(this);
                            if (ec) {
                              fmt::print(stderr, "Write error: {}\n", ec.message());
                            } else {
                              fmt::print("Response sent\n");
                            }
                          });
      } else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
        // 连接关闭，不处理请求
        fmt::print("Connection closed by client: {}\n", ec.message());
      } else {
          fmt::print(stderr, "Read error: {}\n", ec.message());
        }
      }
  );
}