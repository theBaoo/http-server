#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <boost/asio.hpp>
#include <cstddef>
#include <memory>
#include <string>

#include "common/constants.hh"

class HTTPHandler;

class TCPServer {
 public:
  TCPServer(const std::string& address, const std::string& port);
  void start();
  void startWithMultipleThreads(int threads);
  void stop();
  void restart();

 private:
  void do_accept();
  void cleanup();

  size_t max_connections_{MAX_CONNECTIONS};

  boost::asio::io_context        io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  // 防止io_context_退出
  std::shared_ptr<boost::asio::io_context::work> work{
      std::make_shared<boost::asio::io_context::work>(io_context_)};

  std::vector<std::shared_ptr<HTTPHandler>> handlers_;
};

#endif // TCP_SERVER_H