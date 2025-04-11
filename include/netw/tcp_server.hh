#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <boost/asio.hpp>
#include <cstddef>
#include <memory>
#include <string>

#include "boost/asio/ssl/context.hpp"
#include "common/constants.hh"
#include "common/macro.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/http_handler.hh"

class TCPServer {
 public:
  TCPServer(boost::asio::io_context& ctx, const std::string& address, const std::string& port,
            bool ssl_enabled = false);
  void start();
  void startWithMultipleThreads(int threads);
  // 一个低级错误: startWithMultipleThreads中调用了join, 导致第二个tcp server并未启动
  // 人工智能全在瞎扯
  // 但其实二分打印日志可以很快找到问题
  // 调试也可以, 会看到主线程在join中阻塞
  void join();
  void stop();
  void restart();

  template <typename... Args>
  void log(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger(ssl_enabled_ ? "tcp server(443)" : "tcp server(80)")
        .info(message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> message, Args&&... args) {
    Logger::getLogger(ssl_enabled_ ? "tcp server(443)" : "tcp server(80)")
        .error(message, std::forward<Args>(args)...);
  }

 private:
  void do_accept();
  void cleanup();

  // size_t max_connections_{MAX_CONNECTIONS};

  bool ssl_enabled_{false};

  boost::asio::io_context& io_context_;
  // 避免clangd要求提供初始化(因为该对象没有默认构造函数)
  std::optional<boost::asio::ssl::context> ssl_context_;
  boost::asio::ip::tcp::acceptor           acceptor_;

  // 防止io_context_退出
  std::shared_ptr<boost::asio::io_context::work> work{
      std::make_shared<boost::asio::io_context::work>(io_context_)};

  std::vector<std::shared_ptr<HandlerBase>> handlers_;
  // 对于公用io_context的情况, workers可以由main管理
  std::vector<std::thread> workers_;
};

#endif // TCP_SERVER_H