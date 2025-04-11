#ifndef APPLICATION_LOGIN_SERVICE_HH
#define APPLICATION_LOGIN_SERVICE_HH

#include <string>

#include "application/context.hh"
#include "application/service.hh"
#include "buffer/session.hh"

class Authenticator {
 public:
  static auto getInstance() -> Authenticator&;
  auto        authenticate(const std::string& username, const std::string& password) -> bool;
  auto        logout(const std::string& session_id) -> void;

 private:
  Authenticator()  = default;
  ~Authenticator() = default;
  DISALLOW_COPY_AND_MOVE(Authenticator);
};

// 按照我的设计思路, 登陆服务应该继承自Service, 服务业务逻辑
// 但工程上应该抽象出鉴权服务, 独立于业务逻辑Service之外, 也方便后续的扩展
class LoginService : public Service {
 public:
  LoginService();
  ~LoginService() override = default;
  DISALLOW_COPY_AND_MOVE(LoginService);

  static auto getInstance() -> LoginService&;
  // 登陆成功后发送跳转响应, 有客户端负责跳转到新的页面
  auto handle(RequestContext& ctx) -> ResponseContext override;

 private:
  // 为什么使用裸指针? 见Router的RouterTable
  Authenticator*  authenticator_;
  SessionManager* session_manager_;

  std::string valid_path_ = "/";
};

#endif // APPLICATION_LOGIN_SERVICE_HH