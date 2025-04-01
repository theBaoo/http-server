#include "concurrency/thread_id_manager.hh"

#include <iomanip>
#include <string>

std::unordered_map<std::thread::id, int> ThreadIdMapper::id_map_;
int                                      ThreadIdMapper::next_id_;
std::mutex                               ThreadIdMapper::mutex_;

auto ThreadIdMapper::getReadableId() -> std::string {
  std::lock_guard<std::mutex> lock(mutex_);
  std::thread::id             id = std::this_thread::get_id();
  if (id_map_.find(id) == id_map_.end()) {
    id_map_[id] = next_id_++;
  }
  std::ostringstream oss;
  oss << "Thread-" << id_map_[id];
  return oss.str();
}

auto getFormattedThreadId() -> std::string {
  std::ostringstream oss;
  oss << std::setw(1) << std::setfill('0')
      << std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000;
  return "Thread-" + oss.str();
}