#ifndef PROTOCOL_HTTP_HANDLER_H
#define PROTOCOL_HTTP_HANDLER_H

#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <string>

#include "application/context.hh"
#include "application/file_service.hh"
#include "application/route.hh"
#include "common/macro.hh"
#include "logging/logger.hh"
#include "protocol/response.hh"

const std::string END_OF_REQUEST = "\r\n\r\n";

// TODO(thebao): Implement dependency injection for file service
class HTTPHandler : public std::enable_shared_from_this<HTTPHandler> {
 public:
  explicit HTTPHandler(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
      : socket_(std::move(socket)), router_(ServiceFactory::createRouter()) {
    log("HTTPHandler is created from {}, {}", socket_->remote_endpoint().address().to_string(),
        socket_->remote_endpoint().port());
  };
  ~HTTPHandler() {
    socket_->close();
  }
  DISALLOW_COPY_AND_MOVE(HTTPHandler);
  void handleRequest();

  auto isClosed() -> bool {
    return !socket_->is_open();
  }

 private:
  std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  boost::asio::streambuf                        buffer_;
  Router                                        router_;
  RequestContext                                request_ctx_;
  ResponseContext                               response_ctx_;

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

  // 异步读取POST请求体
  auto getBody() -> void;
  auto getBodyWithLength(std::size_t length) -> void;
  auto getBodyWithChunked() -> void;
  //  chunked transfer encoding
  //  auto getBody(RequestContext ctx) -> void;

  void handleOneRequest(const boost::system::error_code& ecd, std::size_t length);

  void processRequest();

  void processBodyComplete();

  void sendResponse(const std::string& response, bool keep_alive);
};

#endif // PROTOCOL_HTTP_HANDLER_H