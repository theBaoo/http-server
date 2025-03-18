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

#endif // COMMON_MACRO_H