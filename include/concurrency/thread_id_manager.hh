#ifndef CONCURRENCY_THREAD_ID_MANAGER_H
#define CONCURRENCY_THREAD_ID_MANAGER_H

#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

class ThreadIdMapper {
 public:
  static auto getReadableId() -> std::string;

 private:
  static std::unordered_map<std::thread::id, int> id_map_;
  static int                                      next_id_;
  static std::mutex                               mutex_;
};

auto getFormattedThreadId() -> std::string;

#endif // CONCURRENCY_THREAD_ID_MANAGER_H