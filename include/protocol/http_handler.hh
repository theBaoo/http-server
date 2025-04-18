#ifndef PROTOCOL_HTTP_HANDLER_H
#define PROTOCOL_HTTP_HANDLER_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/streambuf.hpp>
#include <cstddef>
#include <functional>
#include <istream>
#include <memory>
#include <string>

#include "application/context.hh"
#include "application/file_service.hh"
#include "application/route.hh"
#include "boost/asio/buffers_iterator.hpp"
#include "boost/asio/completion_condition.hpp"
#include "boost/asio/error.hpp"
#include "boost/asio/read.hpp"
#include "boost/system/detail/error_code.hpp"
#include "buffer/session.hh"
#include "common/macro.hh"
#include "logging/logger.hh"
#include "protocol/parser.hh"
#include "protocol/response.hh"
#include "protocol/uri_decoder.hh"

const std::string END_OF_REQUEST = "\r\n\r\n";

// TCP Server通过这个对象管理HTTP(S)请求, 避免冗余的模版声明
// TCP Server负责启动请求处理器, 并在合适的时候关闭连接
class HandlerBase {
 public:
  HandlerBase() = default;
  DISALLOW_COPY_AND_MOVE(HandlerBase);
  virtual ~HandlerBase()          = default;
  virtual auto initiate() -> void = 0;
  virtual auto isClosed() -> bool = 0;
};

// TODO(thebao): Implement dependency injection for file service
template <typename Socket>
class HTTPHandler : public std::enable_shared_from_this<HTTPHandler<Socket>>, public HandlerBase {
 public:
  explicit HTTPHandler(std::shared_ptr<Socket> socket)
      : socket_(std::move(socket)),
        router_(ServiceFactory::createRouter()){
            // 日志逻辑放在这里并不合适
            // 因为需要原始的socket, 而ssl连接的socket并未进入handler
            // 而构造函数又无法简单地模版偏特化
            // 所以将该日志逻辑移动至上一层, TCP Server
            // log("HTTPHandler is created from {}, {}",
            // socket_->remote_endpoint().address().to_string(),
            //     socket_->remote_endpoint().port());
        };

  // 一个低级错误: 双重释放了socket
  // 连接关闭后调用一次close
  // 下一次连接到来后cleanup导致了析构函数的调用
  // 但是这也说明cleanup的资源清理是正确的
  // 另外remote_endpoint()出错会导致整个程序崩溃, 或许可以加入错误处理的考量范围
  ~HTTPHandler() override {
    if (!isclosed_) {
      close(socket_);
    }
  }

  DISALLOW_COPY_AND_MOVE(HTTPHandler);
  auto initiate() -> void override {
    do_initiate(*socket_);
  }

  void handleRequest();

  // 传入socket是为了模版特化, 这样可以自动选择对应的close
  template <typename S>
  auto close(S socket) -> void {
    log("default close");
    socket->close();
    isclosed_ = true;
    log("Socket closed.");
  }

  auto close(
      std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket) // NOLINT
      -> void {
    socket->async_shutdown([this](const boost::system::error_code& ecd) {
      if (!ecd) {
        log("SSL connection shutdown.");
      } else {
        error("SSL connection close error: {} from {} {}", ecd.message(),
              socket_->lowest_layer().remote_endpoint().address().to_string(),
              socket_->lowest_layer().remote_endpoint().port());
      }
      // 安全起见, 直接关闭socket
      socket_->next_layer().close();
      isclosed_ = true;
      log("Socket closed.");
    });
  }

  auto isClosed() -> bool override {
    // return !socket_->is_open();
    // return get_socket(socket_)->is_open();
    return isclosed_;
  }

 private:
  std::shared_ptr<Socket> socket_;
  boost::asio::streambuf  buffer_;
  Router                  router_;
  RequestContext          request_ctx_;
  ResponseContext         response_ctx_;
  bool                    isclosed_{false};

  template <typename... Args>
  void log(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger("http handler").info(message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger("http handler").error(message, std::forward<Args>(args)...);
  }

  template <typename S>
  void do_initiate([[maybe_unused]] S& socket) {
    handleRequest();
  }

  // HTTPS 初始化（执行握手）
  void do_initiate(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket) {
    log("HTTPS connection establishing.");
    socket.async_handshake(boost::asio::ssl::stream_base::server,
                           [this](const boost::system::error_code& ecd) {
                             if (!ecd) {
                               log("SSL handshake completed.");
                               handleRequest();
                             } else {
                               error("SSL handshake error: {}", ecd.message());
                             }
                           });
  }

  auto getFileContent(const std::string& uri) -> std::string;

  auto getRequest() -> std::string;

  // 异步读取POST请求体
  auto getBody() -> void;
  auto getBodyWithLen(std::size_t length) -> void;
  auto getBodyWithLength(std::size_t length) -> void;
  auto getBodyWithChunked() -> void;
  //  chunked transfer encoding
  //  auto getBody(RequestContext ctx) -> void;

  void handleOneRequest(const boost::system::error_code& ecd, std::size_t length);

  void processRequest();
  void processBodyComplete();

  void sendResponse(const std::string& response, bool keep_alive);
  void sendContinue();
  void checkErrorCode(const boost::system::error_code& ecd) {
    if (ecd == boost::asio::error::eof || ecd == boost::asio::error::connection_reset) {
      close(socket_);
    } else {
      error("Async I/O error: {}", ecd.message());
    }
  }
  void read_length(const boost::system::error_code& ecd, size_t length) {
    (void)length;
    if (!ecd) {
      auto        begin = buffers_begin(buffer_.data());
      auto        end   = buffers_end(buffer_.data());
      std::string data(begin, end);
      // 检查是否有ext
      size_t body_size{};
      auto   pos = data.find(';');
      if (pos != std::string::npos) {
        // TODO(thebao): 处理ext
      }

      pos = data.find(CRLF);
      if (pos == std::string::npos) {
        error("Read chunked body: expected CRLF");
        close(socket_);
      }

      body_size = std::stoul(data.substr(0, pos), nullptr, HEX);
      buffer_.consume(pos + 2); // 消耗掉CRLF
      log("Chunked body size: {}", body_size);

      // 传输结束
      if (body_size == 0) {
        // Is it safe to directlt consume the second \r\n?
        async_read_until(*socket_, buffer_, CRLF,
                         [this](const boost::system::error_code& ecd, size_t length) {
                           finish_chunk(ecd, length);
                         });
        return;
      }

      async_read_until(
          *socket_, buffer_, CRLF,
          [this](const boost::system::error_code& ecd, size_t length) { read_block(ecd, length); });
    } else {
      checkErrorCode(ecd);
    }
  }
  void read_block(const boost::system::error_code& ecd, size_t length) {
    (void)length;
    if (!ecd) {
      auto        begin = buffers_begin(buffer_.data());
      auto        end   = buffers_end(buffer_.data());
      std::string data(begin, end);

      auto pos = data.find(CRLF);
      if (pos == std::string::npos) {
        error("Read chunked body: expected CRLF");
        close(socket_);
      }
      request_ctx_.addBody(data.substr(0, pos));
      buffer_.consume(pos + 2); // 消耗掉CRLF

      log("Received chunked body: {}", data);

      // 继续读取下一个块
      async_read_until(*socket_, buffer_, CRLF,
                       [this](const boost::system::error_code& ecd, size_t length) {
                         read_length(ecd, length);
                       });
    } else {
      checkErrorCode(ecd);
    }
  }
  void finish_chunk(const boost::system::error_code& ecd, [[maybe_unused]] size_t length) {
    if (!ecd) {
      auto begin = buffers_begin(buffer_.data());
      if (begin[0] == '\r' && begin[1] == '\n') {
        buffer_.consume(2);
        processBodyComplete();
      } else {
        error("Expected CRLF after last chunk");
        close(socket_);
      }
    } else {
      checkErrorCode(ecd);
    }
  }
};

template <typename Socket>
auto make_handler(std::shared_ptr<Socket> socket) -> std::shared_ptr<HandlerBase> {
  return std::make_shared<HTTPHandler<Socket>>(std::move(socket));
}

// TODO(thebao): Reduce cognitive complexity of handleRequest function
template <typename Socket>
void HTTPHandler<Socket>::handleRequest() {
  boost::asio::async_read_until(*socket_, buffer_, END_OF_REQUEST,
                                [this](const boost::system::error_code& ecd, std::size_t length) {
                                  handleOneRequest(ecd, length);
                                });
}

template <typename Socket>
void HTTPHandler<Socket>::handleOneRequest(const boost::system::error_code& ecd,
                                           std::size_t                      length) {
  using boost::asio::error::connection_reset;
  using boost::asio::error::eof;
  using boost::system::error_code;
  log("Received {} bytes", length);

  if (!ecd) {
    processRequest();
  } else if (ecd == eof || ecd == connection_reset) {
    // 连接关闭，不处理请求
    close(socket_);
  } else {
    error("Read error: {}", ecd.message());
  }
}

template <typename Socket>
auto HTTPHandler<Socket>::getRequest() -> std::string {
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
template <typename Socket>
auto HTTPHandler<Socket>::getBody() -> void {
  auto content_length    = request_ctx_.getHeader("Content-Length");
  auto transfer_encoding = request_ctx_.getHeader("Transfer-Encoding");

  if (content_length.empty() && transfer_encoding == "chunked") {
    log("Chunked transfer encoding detected.");
    getBodyWithChunked();
  } else if (!content_length.empty()) {
    std::size_t length = std::stoul(content_length);
    log("Expected body length {}", length);
    getBodyWithLen(length);
  } else {
    processBodyComplete();
  }
}

template <typename Socket>
auto HTTPHandler<Socket>::getBodyWithLength(std::size_t length) -> void {
  using boost::asio::async_read;
  using boost::asio::buffer;
  using boost::asio::buffers_begin;
  using boost::asio::buffers_end;
  using boost::asio::transfer_at_least;
  using boost::system::error_code;

  std::function<void(const error_code&, size_t)> handler =
      [this, length, &handler](const error_code& ecd, std::size_t bytes_transferred) mutable {
        if (!ecd) {
          log("Received {} bytes", bytes_transferred);
          buffer_.commit(bytes_transferred);
          auto buffers = buffer_.data();
          std::string data(boost::asio::buffers_begin(buffers),
                             boost::asio::buffers_begin(buffers) + static_cast<std::ptrdiff_t>(bytes_transferred));
          // std::string data(bytes_transferred, '\0');                    // 正确分配
          // boost::asio::buffer_copy(boost::asio::buffer(data), buffers); // 安全拷贝
          buffer_.consume(bytes_transferred);
        
          log("buffer size: {}", buffer_.size());
          request_ctx_.addBody(data);
          log("Received body: {}", data);
          size_t received = request_ctx_.getBody().size();
          log("Received body size: {}", received);
          if (received < length) {
            // 继续读取剩余部分
            log("Continue reading body...");
            async_read(*socket_, buffer_, transfer_at_least(1), handler);
          } else {
            processBodyComplete();
          }
        } else {
          if (ecd == boost::asio::error::eof || ecd == boost::asio::error::connection_reset) {
            close(socket_);
          } else {
            error("Read error: {}", ecd.message());
          }
        }
      };

  log("Before async_read, buffer size: {}", buffer_.size());
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

template <typename Socket>
auto HTTPHandler<Socket>::getBodyWithLen(std::size_t elength) -> void {
  using boost::asio::async_read;
  using boost::asio::transfer_at_least;
  using boost::system::error_code;
  auto self = this->shared_from_this();

  async_read(*socket_, buffer_, transfer_at_least(1),
    [this, self, elength](const error_code& ecd, size_t length) {
    if (!ecd) {
      log("Received {} bytes", length);
      buffer_.commit(length);
      auto buffers = buffer_.data();
      std::string data(boost::asio::buffers_begin(buffers),
                        boost::asio::buffers_begin(buffers) + static_cast<std::ptrdiff_t>(length));
      buffer_.consume(length);
      request_ctx_.addBody(data);
      log("Received body: {}", data);

      auto received = request_ctx_.getBody().size();
      if (elength > received) {
        getBodyWithLen(elength);
      } else if (elength == received) {
        processBodyComplete();
      } else {
        error("Received body size {} exceeds expected size {}", received, elength);
        close(socket_);
      }
    } else {
      checkErrorCode(ecd);
    }
  });
}

// async_read_until: 按分隔符读取
// async_read: 按数据本身读取, 如(至少)读取一定量的数据; 且在回调中需额外截取数据
template <typename Socket>
auto HTTPHandler<Socket>::getBodyWithChunked() -> void {
  using boost::asio::async_read;
  using boost::asio::buffer;
  using boost::asio::buffers_begin;
  using boost::asio::buffers_end;
  using boost::asio::transfer_at_least;
  using boost::asio::transfer_exactly;
  using boost::system::error_code;
  // TODO(thebao): 处理chunked transfer encoding
  async_read_until(*socket_, buffer_, CRLF,
                   [this](const boost::system::error_code& ecd, std::size_t length) {
                     read_length(ecd, length);
                   });
}

// TODO(thebao): 处理异步post请求体
template <typename Socket>
void HTTPHandler<Socket>::processRequest() {
  // ----------------- Request -----------------
  std::string request = getRequest();
  request_ctx_        = Parser::parse(request);
  // ----------------- Request -----------------
  if (request_ctx_.getMethod() == "POST") {
    if (request_ctx_.getHeader("Expect") == "100-continue") {
      sendContinue();
    }
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

template <typename Socket>
void HTTPHandler<Socket>::processBodyComplete() {
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

template <typename Socket>
void HTTPHandler<Socket>::sendResponse(const std::string& response, bool keep_alive) {
  using boost::asio::async_write;
  using boost::asio::buffer;
  using boost::system::error_code;

  auto self = this->shared_from_this();
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
                    close(socket_);
                  }
                }
              });
}

template <typename Socket>
void HTTPHandler<Socket>::sendContinue() {
  using boost::asio::buffer;
  using boost::asio::write;
  using boost::system::error_code;

  std::string msg = "HTTP/1.1 100 Continue\r\n\r\n";

  auto self = this->shared_from_this();
  write(*socket_, buffer(msg));
  log("Ready to receive body.");
}

#endif // PROTOCOL_HTTP_HANDLER_H