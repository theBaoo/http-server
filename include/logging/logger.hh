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
#include "fmt/color.h"

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
  virtual bool supportsColor() const            = 0;  // 新增接口
  DISALLOW_COPY_AND_MOVE(LogSink);
};

class ConsoleSink : public LogSink {
 public:
  void write(const std::string& message) override {
    fmt::println("{}", message);
  }
  void shutdown() override {}
  bool supportsColor() const override { return true; }  // 控制台支持颜色
};

class FileSink : public LogSink {
 public:
  explicit FileSink(const std::string& filename) : file_(filename, std::ios::app) {}
  
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

  bool supportsColor() const override { return false; }  // 文件不支持颜色

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

    std::string now = getCurrentDateTime();
    std::string level_str = getLogLevelString(level);
    std::string plain_message = fmt::format(format, std::forward<Args>(args)...);

    // 纯文本消息
    std::string message = fmt::format("[{}] [{}] {}: {}", now, level_str, name_, plain_message);

    // 选择日志级别颜色
    fmt::text_style level_style;
    switch (level) {
        case LogLevel::INFO:  level_style = fg(fmt::color::green); break;
        case LogLevel::ERROR: level_style = fg(fmt::color::red); break;
        case LogLevel::WARN:  level_style = fg(fmt::color::yellow); break;
        case LogLevel::DEBUG: level_style = fg(fmt::color::blue); break;
        case LogLevel::TRACE: level_style = fg(fmt::color::cyan); break;
        case LogLevel::FATAL: level_style = fg(fmt::color::purple); break;
    }
    fmt::text_style name_style = fg(fmt::color::light_coral);

    // 带颜色的消息：使用 fmt::format 并应用样式
    std::string colored_message = fmt::format(
        "[{}] [{}] {}: {}", 
        now,
        fmt::format(level_style, "{}", level_str),           // 应用级别颜色
        fmt::format(name_style, "{}", name_),              // 应用名称颜色
        plain_message                                        // 消息内容保持无色
    );

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& sink : sinks_) {
        sink->write(sink->supportsColor() ? colored_message : message);
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