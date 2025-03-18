#ifndef PROTOCOL_HTTP_HANDLER_H
#define PROTOCOL_HTTP_HANDLER_H

#include <string>

#include "asio.hpp"
#include "asio/streambuf.hpp"
#include "common/macro.hh"
#include "protocol/response.hh"

const std::string END_OF_REQUEST = "\r\n\r\n";

class HTTPHandler : public std::enable_shared_from_this<HTTPHandler> {
 public:
  explicit HTTPHandler(std::shared_ptr<asio::ip::tcp::socket> socket)
      : socket_(std::move(socket)){};
  ~HTTPHandler() {
    socket_->close();
  }
  DISALLOW_COPY_AND_MOVE(HTTPHandler);
  void handleRequest();

 private:
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  asio::streambuf                        buffer_;
};

#endif // PROTOCOL_HTTP_HANDLER_H