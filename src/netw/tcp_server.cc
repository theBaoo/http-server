#include "netw/tcp_server.hh"
#include <iostream>
#include "fmt/core.h"
#include "protocol/http_handler.hh"

TCPServer::TCPServer(const std::string& address, const std::string& port)
    : acceptor_(io_context_,
                asio::ip::tcp::endpoint(asio::ip::make_address(address), std::stoi(port))) {
}

void TCPServer::start() {
  do_accept();
  io_context_.run();
}

void TCPServer::do_accept() {
  auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
  acceptor_.async_accept(*socket, [this, socket](const asio::error_code& ecd) {
    if (!ecd) {
      fmt::print("Client connected: {}\n", socket->remote_endpoint().address().to_string());
      auto handler = std::make_shared<HTTPHandler>(socket);
      handlers_.push_back(handler);
      handler->handleRequest();
    } else {
      fmt::print(stderr, "Accept error: {}\n", ecd.message());
    }
    do_accept();
  });
}