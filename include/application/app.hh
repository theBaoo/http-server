#ifndef APPLICATION_APP_HH
#define APPLICATION_APP_HH

#include <fmt/core.h>

#include <boost/asio/io_context.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "common/constants.hh"
#include "common/macro.hh"
#include "logging/logger.hh"
#include "tcp_server.hh"

class App {
 public:
  App();
  ~App();
  DISALLOW_COPY_AND_MOVE(App);

  ENABLE_INFO("App")
  ENABLE_ERROR("App")

  void start(int num_threads = 4);
  void stop();

  [[nodiscard]] auto isRunning() const {
    return running_;
  }

 private:
  std::shared_ptr<boost::asio::io_context> ctx_;
  std::unique_ptr<TCPServer>               http_server_;     // HTTP server
  std::unique_ptr<TCPServer>               https_server_;    // HTTPS server
  std::vector<std::thread>                 threads_;         // Thread pool for ctx_.run()
  bool                                     running_ = false; // Tracks server state
};

#endif // APPLICATION_APP_HH