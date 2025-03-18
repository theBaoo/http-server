#ifndef LOGGING_LOGGER_H
#define LOGGING_LOGGER_H

#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "common/constants.hh"
#include "common/macro.hh"
#include "fmt/core.h"

enum class LogLevel {
  INFO,
  ERROR,
  WARN,
  DEBUG,
  TRACE,
  FATAL,
};

auto getLogLevelString(LogLevel level) -> std::string;

auto getCurrentDateTime() -> std::string;

class LogSink {
 public:
  LogSink()                                      = default;
  virtual ~LogSink()                             = default;
  virtual void write(const std::string& message) = 0;
  virtual void shutdown()                        = 0;
  DISALLOW_COPY_AND_MOVE(LogSink);
};

class ConsoleSink : public LogSink {
 public:
  void write(const std::string& message) override {
    fmt::println("{}", message);
  }
  void shutdown() override {
  }
};

class FileSink : public LogSink {
 public:
  explicit FileSink(const std::string& filename) : file_(filename, std::ios::app) {
  }
  void write(const std::string& message) override {
    if (file_.is_open()) {
      file_ << message << std::endl;
    }
  }
  void shutdown() override {
    if (file_.is_open()) {
      file_.close();
    }
  }

 private:
  std::ofstream file_;
};

class Logger {
 public:
  static auto getLogger(const std::string& name = "") -> Logger&;

  void addSink(std::unique_ptr<LogSink> sink);

  template <typename... Args>
  void log(LogLevel level, fmt::format_string<Args...> format, Args&&... args) {
    if (level < level_) {
      return;
    }

    std::string now       = getCurrentDateTime();
    std::string level_str = getLogLevelString(level);

    std::string message = fmt::format("[{}] [{}] {}: {}", now, level_str, name_,
                                      fmt::format(format, std::forward<Args>(args)...));

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& sink : sinks_) {
      sink->write(message);
    }
  }

  void setName(const std::string& name) {
    name_ = name;
  }

  void setLevel(const LogLevel level) {
    level_ = level;
  }

  template <typename... Args>
  void info(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::INFO, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::ERROR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::WARN, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void debug(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void trace(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::TRACE, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void fatal(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::FATAL, format, std::forward<Args>(args)...);
  }

  void shutdown();

  DISALLOW_COPY_AND_MOVE(Logger);

 private:
  Logger() {
    auto log_file = root;
    log_file += "/logs/log_";
    log_file += getCurrentDateTime();
    log_file += ".log";
    addSink(std::make_unique<ConsoleSink>());
    addSink(std::make_unique<FileSink>(log_file));
  }

  ~Logger() {
    shutdown();
  }

  std::vector<std::unique_ptr<LogSink>> sinks_;
  std::string                           name_;
  LogLevel                              level_ = LogLevel::INFO;
  std::mutex                            mutex_;
};

#endif // LOGGING_LOGGER_H