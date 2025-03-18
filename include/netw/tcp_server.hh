#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <asio.hpp>
#include <memory>
#include <string>

class HTTPHandler;

class TCPServer {
 public:
  TCPServer(const std::string& address, const std::string& port);
  void start();

 private:
  void do_accept();

  asio::io_context        io_context_;
  asio::ip::tcp::acceptor acceptor_;

  std::vector<std::shared_ptr<HTTPHandler>> handlers_;
};

#endif // TCP_SERVER_H