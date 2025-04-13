#ifndef BUFFER_SESSION_HH
#define BUFFER_SESSION_HH

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

#include "common/macro.hh"

// 用于Cookie机制, 实现典型的网站登录
class Session {
  using Clock     = std::chrono::system_clock; // 改为 system_clock 以便与时间格式兼容
  using TimePoint = Clock::time_point;

 public:
  [[nodiscard]] auto to_string() const -> std::string;
  auto               from_string(const std::string& session_str) -> void;

  DEFINE_GETTER_AND_SETTER(std::string, SessionId, session_id_)
  DEFINE_GETTER_AND_SETTER(std::string, Username, username_)
  DEFINE_GETTER_AND_SETTER(TimePoint, ExpireAt, expire_at_)

  [[nodiscard]] auto getExpireAtStr() const -> std::string {
    return time_point_to_string(expire_at_);
  }

 private:
  std::string session_id_;
  std::string username_;
  TimePoint   expire_at_; // 过期时间

  static auto time_point_to_string(const TimePoint& tpt) -> std::string;
  static auto string_to_time_point(const std::string& time_str) -> TimePoint;
};

// TODO(thebao): 持久化
class SessionManager {
 public:
  SessionManager();
  ~SessionManager();
  DISALLOW_COPY_AND_MOVE(SessionManager);
  static auto getInstance() -> SessionManager&;
  // if success, return cookie
  // id + expire time
  auto create_session() -> std::tuple<std::string, std::string>;
  // if expired or not found, return empty
  auto get_sesstion(const std::string& sid) -> std::optional<Session>;
  void remove_expired_sessions(); // 其实不需要主动清理, 只需要在session的检查中顺便清理.
                                  // 但是可能会挤压大量的数据, 所以还是得考虑定时清理
  void load_from_file();
  void save_to_file();

 private:
  using SessionTable = std::unordered_map<std::string, Session>;
  SessionTable session_table_;

  static auto generate_id() -> std::string {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
  }
};

#endif // BUFFER_SESSION_HH