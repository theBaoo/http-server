#include "netw/tcp_server.hh"

// #include <__algorithm/remove_if.h>
#include <algorithm>
#include <iostream>

#include "application/file_service.hh"
#include "boost/asio/io_context.hpp"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "protocol/http_handler.hh"

TCPServer::TCPServer(boost::asio::io_context& ctx, const std::string& address,
                     const std::string& port, bool ssl_enabled)
    : io_context_(ctx),
      acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address),
                                                            std::stoi(port))) {
  if (ssl_enabled) {
    ssl_enabled_ = true;
    ssl_context_.emplace(boost::asio::ssl::context::tlsv12_server);
    // 使用缓冲区读取出错, 故直接读取文件
    try {
      ssl_context_->use_certificate_file(cert, boost::asio::ssl::context::pem);
      ssl_context_->use_private_key_file(key, boost::asio::ssl::context::pem);
    } catch (const std::exception& e) {
      log("SSL error: {}", e.what());
      throw;
    }
    log("TCP Server for HTTPS is created");
  } else {
    log("TCP Server for HTTP is created");
  }
}

void TCPServer::start() {
  do_accept();
  // io_context_.run();
}

void TCPServer::startWithMultipleThreads(int threads) {
  do_accept();
  workers_.reserve(threads);
  for (int i = 0; i < threads; ++i) {
    workers_.emplace_back([this] { io_context_.run(); });
  }
}

void TCPServer::join() {
  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
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
  using boost::asio::ip::tcp;
  using boost::system::error_code;
  auto socket = std::make_shared<tcp::socket>(io_context_);
  acceptor_.async_accept(*socket, [this, socket](const error_code& ecd) {
    if (!ecd) {
      // auto handler = std::make_shared<HTTPHandler>(socket);
      log("HTTP(S) Handler is created from {}, {}", socket->remote_endpoint().address().to_string(),
          socket->remote_endpoint().port());
      std::shared_ptr<HandlerBase> handler;
      if (!ssl_enabled_) {
        handler = make_handler(socket);
      } else {
        auto ssl_socket = std::make_shared<boost::asio::ssl::stream<tcp::socket>>(
            std::move(*socket), *ssl_context_);
        handler = make_handler(ssl_socket);
      }
      handlers_.push_back(handler);
      handler->initiate();
    } else {
      error("Accept error: {}", ecd.message());
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
  log("Cleanup {} handlers", count);
}