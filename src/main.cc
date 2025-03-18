#include <iostream>
#include "fmt/core.h"
#include "tcp_server.hh"

#include "logging/logger.hh"

auto main() -> int {
  fmt::println("HTTP Server is running on http://localhost:8080");
  fmt::println("Getting logger.");
  Logger::getLogger("HTTP Server").info("HTTP Server is running on http://localhost:8080");
  TCPServer server("0.0.0.0", "8080");
  server.start();

  Logger::getLogger().shutdown();
  return 0;
}