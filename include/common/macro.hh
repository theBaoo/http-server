#ifndef COMMON_MACRO_H
#define COMMON_MACRO_H

#define DEFAULT_COPY_AND_MOVE(classname)                  \
  classname(const classname&)                  = default; \
  auto operator=(const classname&)->classname& = default; \
  classname(classname&&)                       = default; \
  auto operator=(classname&&)->classname&      = default

#define DISALLOW_COPY_AND_MOVE(classname)                \
  classname(const classname&)                  = delete; \
  auto operator=(const classname&)->classname& = delete; \
  classname(classname&&)                       = delete; \
  auto operator=(classname&&)->classname&      = delete

#define ENABLE_INFO(module)                                               \
  template <typename... Args>                                             \
  void log(fmt::format_string<Args...> message, Args&&... args) {         \
    Logger::getLogger(module).info(message, std::forward<Args>(args)...); \
  }

#define ENABLE_ERROR(module)                                               \
  template <typename... Args>                                              \
  void error(fmt::format_string<Args...> message, Args&&... args) {        \
    Logger::getLogger(module).error(message, std::forward<Args>(args)...); \
  }

#define ENABLE_STATIC_INFO(module)                                        \
  template <typename... Args>                                             \
  static void log(fmt::format_string<Args...> message, Args&&... args) {  \
    Logger::getLogger(module).info(message, std::forward<Args>(args)...); \
  }

#define ENABLE_STATIC_ERROR(module)                                        \
  template <typename... Args>                                              \
  static void error(fmt::format_string<Args...> message, Args&&... args) { \
    Logger::getLogger(module).error(message, std::forward<Args>(args)...); \
  }

#endif // COMMON_MACRO_H