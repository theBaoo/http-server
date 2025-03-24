#include "logging/logger.hh"

#include <fstream>

#include "fmt/core.h"

// auto getLogLevelString(const LogLevel level) -> std::string {
//   switch (level) {
//     case LogLevel::INFO:
//       return "INFO";
//     case LogLevel::ERROR:
//       return "ERROR";
//     case LogLevel::WARN:
//       return "WARN";
//     case LogLevel::DEBUG:
//       return "DEBUG";
//     case LogLevel::TRACE:
//       return "TRACE";
//     case LogLevel::FATAL:
//       return "FATAL";
//   }
//   return "UNKNOWN";
// }
auto getLogLevelString(LogLevel level) -> std::string {
  static const std::map<LogLevel, std::string> levelStrings = {
      {LogLevel::INFO, "INFO"},   {LogLevel::ERROR, "ERROR"}, {LogLevel::WARN, "WARN"},
      {LogLevel::DEBUG, "DEBUG"}, {LogLevel::TRACE, "TRACE"}, {LogLevel::FATAL, "FATAL"}};
  return levelStrings.at(level);
}

auto getCurrentDateTime() -> std::string {
  auto               now      = std::chrono::system_clock::now();
  std::time_t        now_time = std::chrono::system_clock::to_time_t(now);
  std::tm            now_tm   = *std::localtime(&now_time);
  std::ostringstream oss;
  oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

// 线程安全问题
// 考虑添加一个队列，将日志消息放入队列，然后由一个线程负责写入文件
auto Logger::getLogger(const std::string& name) -> Logger& {
  static Logger instance;
  if (!name.empty()) {
    instance.setName(name);
  }
  return instance;
}

void Logger::addSink(std::unique_ptr<LogSink> sink) {
  std::lock_guard<std::mutex> lock(mutex_);
  sinks_.push_back(std::move(sink));
}

void Logger::shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& sink : sinks_) {
    sink->shutdown();
  }
  sinks_.clear();
}