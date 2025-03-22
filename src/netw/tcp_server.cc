#include "netw/tcp_server.hh"

#include <iostream>

#include "application/file_service.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/http_handler.hh"

TCPServer::TCPServer(const std::string& address, const std::string& port)
    : acceptor_(io_context_,
                asio::ip::tcp::endpoint(asio::ip::make_address(address), std::stoi(port))) {
  Logger::getLogger("tcp server").info("TCP server is created.");
}

void TCPServer::start() {
  do_accept();
  io_context_.run();
}

void TCPServer::do_accept() {
  auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
  acceptor_.async_accept(*socket, [this, socket](const asio::error_code& ecd) {
    if (!ecd) {
      auto handler = std::make_shared<HTTPHandler>(socket, FileService::getInstance());
      handlers_.push_back(handler);
      handler->handleRequest();
    } else {
      Logger::getLogger("tcp server").error("Accept error: {}", ecd.message()); 
    }
    do_accept();
  });
}