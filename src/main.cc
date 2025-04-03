#include <iostream>

#include "boost/asio/io_context.hpp"
#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "tcp_server.hh"

auto main() -> int {
  Logger::getLogger("HTTP Server").info("HTTP(S) Server is running on http://localhost:8080(443)");

  // boost::asio::io_context http_ctx;
  // boost::asio::io_context https_ctx;
  boost::asio::io_context ctx;
  TCPServer               http_server(ctx, ADDR, PORT);
  TCPServer               https_server(ctx, ADDR, SSL_PORT, true);
  // server.start();
  http_server.startWithMultipleThreads(4);
  https_server.startWithMultipleThreads(4);
  // ctx.run();
  http_server.join();
  https_server.join();

  Logger::getLogger().shutdown();
  return 0;
}