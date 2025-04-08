#include "buffer/session.hh"

#include <string>

[[nodiscard]] auto Session::to_string() const -> std::string {
  return session_id_ + ":" + username_ + ", " + time_point_to_string(expire_at_);
}

auto Session::from_string(const std::string& session_str) -> void {
  // 假设格式为 "session_id:username, YYYY-MM-DD HH:MM:SS"
  auto pos1 = session_str.find(':');
  auto pos2 = session_str.find(", ");
  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    throw std::invalid_argument("Invalid session string format");
  }

  session_id_ = session_str.substr(0, pos1);
  username_   = session_str.substr(pos1 + 1, pos2 - pos1 - 1);
  expire_at_  = string_to_time_point(session_str.substr(pos2 + 2));
}

auto Session::time_point_to_string(const TimePoint& tpt) -> std::string {
  std::time_t        time = Clock::to_time_t(tpt);
  std::tm            tm_  = *std::localtime(&time); // 转换为本地时间
  std::ostringstream oss;
  oss << std::put_time(&tm_, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

auto Session::string_to_time_point(const std::string& time_str) -> TimePoint {
  std::istringstream iss(time_str);
  std::tm            tm_ = {};
  iss >> std::get_time(&tm_, "%Y-%m-%d %H:%M:%S");
  if (iss.fail()) {
    throw std::invalid_argument("Invalid time format");
  }
  std::time_t time = std::mktime(&tm_);
  return Clock::from_time_t(time);
}

auto SessionManager::getInstance() -> SessionManager& {
  static SessionManager instance;
  return instance;
}

auto SessionManager::create_session() -> std::string {
  using Clock = std::chrono::system_clock;
  Session session;
  // TODO(thebao): 这里应该使用UUID来生成session_id
  session.setSessionId("session_" + std::to_string(session_table_.size() + 1));
  session.setUsername("user_" + std::to_string(session_table_.size() + 1));
  session.setExpireAt(Clock::now() + std::chrono::hours(1)); // 设置过期时间为1小时后
  session_table_[session.getSessionId()] = session;
  return session.getSessionId();
}

auto SessionManager::get_sesstion(const std::string& sid) -> std::optional<Session> {
  auto itr = session_table_.find(sid);
  if (itr != session_table_.end()) {
    return itr->second;
  }
  return std::nullopt;
}