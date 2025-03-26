#include "netw/tcp_server.hh"

// #include <__algorithm/remove_if.h>
#include <algorithm>
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

void TCPServer::startWithMultipleThreads(int threads) {
  do_accept();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for (int i = 0; i < threads; ++i) {
    workers.emplace_back([this] { io_context_.run(); });
  }
  for (auto& worker : workers) {
    worker.join();
  }
}

void TCPServer::stop() {
  io_context_.stop();
}

void TCPServer::restart() {
  io_context_.reset();
  start();
}

void TCPServer::do_accept() {
  auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
  acceptor_.async_accept(*socket, [this, socket](const asio::error_code& ecd) {
    if (!ecd) {
      auto handler = std::make_shared<HTTPHandler>(socket);
      handlers_.push_back(handler);
      handler->handleRequest();
    } else {
      Logger::getLogger("tcp server").error("Accept error: {}", ecd.message());
    }
    do_accept();
    cleanup();
  });
}

void TCPServer::cleanup() {
  auto itr   = std::remove_if(handlers_.begin(), handlers_.end(),
                              [](const auto& handler) { return handler->isClosed(); });
  auto count = std::distance(itr, handlers_.end());
  handlers_.erase(itr, handlers_.end());
  Logger::getLogger("tcp server").info("Cleanup {} handlers", count);
}