#ifndef PROTOCOL_HTTP_HANDLER_H
#define PROTOCOL_HTTP_HANDLER_H

#include <string>

#include "application/file_service.hh"
#include "asio.hpp"
#include "asio/streambuf.hpp"
#include "common/macro.hh"
#include "logging/logger.hh"
#include "protocol/response.hh"

const std::string END_OF_REQUEST = "\r\n\r\n";

// TODO(thebao): Implement dependency injection for file service
class HTTPHandler : public std::enable_shared_from_this<HTTPHandler> {
 public:
  explicit HTTPHandler(std::shared_ptr<asio::ip::tcp::socket> socket, FileService& fileService)
      : socket_(std::move(socket)), fileService_(fileService) {
    log("HTTPHandler is created from {}, {}", socket_->remote_endpoint().address().to_string(), socket_->remote_endpoint().port());
  };
  ~HTTPHandler() {
    socket_->close();
  }
  DISALLOW_COPY_AND_MOVE(HTTPHandler);
  void handleRequest();

 private:
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  asio::streambuf                        buffer_;
  FileService&                           fileService_;
  

  template <typename... Args>
  void log(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger("http handler").info(message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger("http handler").error(message, std::forward<Args>(args)...);
  }

  auto getFileContent(const std::string& uri) -> std::string;

  auto getRequest() -> std::string;

  void handleOneRequest(const asio::error_code& ecd, std::size_t length);

  void processRequest();

  void sendResponse(const std::string& response, bool keep_alive);
};

#endif // PROTOCOL_HTTP_HANDLER_H