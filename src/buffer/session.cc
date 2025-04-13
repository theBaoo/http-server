#include "buffer/session.hh"

#include <fstream>
#include <string>

#include "common/constants.hh"

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
  std::tm            tm_  = *std::gmtime(&time); // 转换为 UTC 时间
  std::ostringstream oss;
  oss << std::put_time(&tm_, "%a, %d %b %Y %H:%M:%S GMT");
  return oss.str();
}

auto Session::string_to_time_point(const std::string& time_str) -> TimePoint {
  std::istringstream iss(time_str);
  std::tm            tm_ = {};
  iss >> std::get_time(&tm_, "%a, %d %b %Y %H:%M:%S GMT");
  if (iss.fail()) {
    throw std::invalid_argument("Invalid time format");
  }
  std::time_t time = timegm(&tm_); // 使用 timegm 将 UTC 时间转换为 time_t
  return Clock::from_time_t(time);
}

SessionManager::SessionManager() {
  load_from_file();
}

SessionManager::~SessionManager() {
  save_to_file();
}

auto SessionManager::getInstance() -> SessionManager& {
  static SessionManager instance;
  return instance;
}

auto SessionManager::create_session() -> std::tuple<std::string, std::string> {
  using Clock = std::chrono::system_clock;
  Session session;
  // TODO(thebao): 这里应该使用UUID来生成session_id
  auto sid = generate_id();
  session.setSessionId(sid);
  session.setUsername("user_" + std::to_string(session_table_.size() + 1));
  session.setExpireAt(Clock::now() + std::chrono::hours(1)); // 设置过期时间为1小时后
  session_table_[session.getSessionId()] = session;
  return {session.getSessionId(), session.getExpireAtStr()};
}

auto SessionManager::get_sesstion(const std::string& sid) -> std::optional<Session> {
  using Clock = std::chrono::system_clock;
  auto itr    = session_table_.find(sid);
  if (itr != session_table_.end()) {
    if (itr->second.getExpireAt() <= Clock::now()) {
      session_table_.erase(itr); // 删除过期的session
    } else {
      return itr->second;
    }
  }
  return std::nullopt;
}

void SessionManager::remove_expired_sessions() {
  using Clock = std::chrono::system_clock;
  auto now    = Clock::now();
  for (auto itr = session_table_.begin(); itr != session_table_.end();) {
    if (itr->second.getExpireAt() <= now) {
      itr = session_table_.erase(itr); // 删除过期的session
    } else {
      ++itr;
    }
  }
}

void SessionManager::load_from_file() {
  std::ifstream ifs(cookie);
  if (ifs.is_open()) {
    std::string line;
    while (std::getline(ifs, line)) {
      Session session;
      session.from_string(line);
      if (session.getSessionId().empty()) {
        continue; // 跳过无效的session
      }
      if (session.getExpireAt() <= std::chrono::system_clock::now()) {
        continue; // 跳过过期的session
      }
      session_table_[session.getSessionId()] = session;
    }
  } else {
    throw std::runtime_error("Failed to open session file");
  }
  ifs.close();
}

// 每次写都需要打开文件, 优化?
void SessionManager::save_to_file() {
  std::ofstream ofs(cookie, std::ios::trunc);
  if (ofs.is_open()) {
    for (const auto& [_, session] : session_table_) {
      ofs << session.to_string() << std::endl;
    }
  } else {
    throw std::runtime_error("Failed to open session file");
  }
  ofs.close();
}
