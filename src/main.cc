#include <sys/signal.h>

#include <csignal>
#include <iostream>

#include "application/app.hh"
#include "boost/asio/io_context.hpp"
#include "common/constants.hh"
#include "fmt/core.h"
#include "logging/logger.hh"
#include "tcp_server.hh"

volatile sig_atomic_t g_shutdown = 0; // NOLINT
volatile sig_atomic_t g_restart  = 0; // NOLINT
void                  signal_handler(int signal) {
                   switch (signal) {
                     case SIGINT:
      Logger::getLogger("main thread").info("Received signal to shut down.");
      g_shutdown = 1;
      break;

                     case SIGUSR1:
      Logger::getLogger("main thread").info("Received signal to restart.");
      g_restart = 1;
      break;

                     default:
      Logger::getLogger("main thread").info("Received unknown signal: {}", signal);
      break;
  }
}

auto main() -> int {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGUSR1, signal_handler);

  try {
    do {
      g_restart  = 0;
      g_shutdown = 0;

      App app;
      app.start();

      while (g_shutdown == 0 && g_restart == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      // 析构函数会调用stop
      // app.stop();
    } while (g_restart == 1);

    Logger::getLogger().shutdown();
  } catch (const std::exception& e) {
    fmt::print(stderr, "Error: {}\n", e.what());
    return 1;
  } catch (...) {
    fmt::print(stderr, "Unknown error occurred.\n");
    return 1;
  }
  return 0;
}