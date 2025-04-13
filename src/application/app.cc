#include "application/app.hh"

#include "logging/logger.hh"

App::App()
    : ctx_(std::make_shared<boost::asio::io_context>()),
      http_server_(std::make_unique<TCPServer>(*ctx_, ADDR, PORT)),
      https_server_(std::make_unique<TCPServer>(*ctx_, ADDR, SSL_PORT, true)) {
  log("App is created!");
}

App::~App() {
  stop();
  log("App is destroyed!");
}

void App::start(int num_threads) {
  if (running_) {
    log("App is already running!");
    return;
  }

  ctx_->restart();

  running_ = true;

  http_server_->start();
  https_server_->start();

  for (int i = 0; i < num_threads; ++i) {
    threads_.emplace_back([this]() { ctx_->run(); });
  }

  log("App started with {} threads!", num_threads);
}

void App::stop() {
  // if (!running_) {
  //   log("App is not running!");
  //   return;
  // }

  running_ = false;

  ctx_->stop();
  for (auto& thread : threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  threads_.clear();

  http_server_->stop();
  https_server_->stop();

  log("App stopped!");
}