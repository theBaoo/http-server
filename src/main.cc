#include <iostream>

#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "tcp_server.hh"

auto main() -> int {
  Logger::getLogger("HTTP Server").info("HTTP Server is running on http://localhost:8080");

  TCPServer server(ADDR, PORT);
  server.start();

  Logger::getLogger().shutdown();
  return 0;
}