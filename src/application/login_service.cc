#include "application/login_service.hh"

#include <string>

#include "application/context.hh"
#include "application/file_service.hh"
#include "application/upload_service.hh"
#include "logging/logger.hh"
#include "protocol/parser.hh"

LoginService::LoginService()
    : authenticator_(&Authenticator::getInstance()),
      session_manager_(&SessionManager::getInstance()) {
}

auto LoginService::getInstance() -> LoginService& {
  static LoginService instance;
  return instance;
}

auto LoginService::handle(RequestContext& ctx) -> ResponseContext {
  ResponseContext res;
  if (ctx.getMethod() == "GET") {
    res.setBody(FileService::getInstance().getFileContent("/login.html"));
    res.setStatusCode(StatusCode::OK);
    res.setStatusMessage("OK");
    res.addHeader("Content-Type", "text/html");
    res.addHeader("Content-Length", std::to_string(res.getBody()->size()));
    return res;
  }

  // POST
  // Cookie: sessionid=abc123
  auto cookie = ctx.getHeader("Cookie");
  if (!cookie.empty() && cookie.find("sessionid=") != std::string::npos) {
    std::string sid     = cookie.substr(cookie.find('=') + 1);
    auto        session = session_manager_->get_sesstion(sid);
    if (session.has_value()) {
      return redirect(res, "/dashboard");
    }
  }

  // auto body     = ctx.getBody();
  // auto params   = Parser::parseBody(body, ContentType::FORM);
  // auto username = params["username"];
  // auto password = params["password"];
  std::string username;
  std::string password;
  auto        ctype = ctx.getHeader("Content-Type");
  if (ctype.contains("multipart/form-data")) {
    auto boundary = ctype.substr(ctype.find("boundary=") + 9);
    auto forms    = parseMultipartFormData(ctx.getBody(), boundary);

    for (const auto& form : forms) {
      if (form.name == "username") {
        username = form.content;
      } else if (form.name == "password") {
        password = form.content;
      }
    }
  }
  Logger::getLogger("login service").info("username: {}, password: {}", username, password);
  auto valid = authenticator_->authenticate(username, password);
  if (!valid) {
    res.setStatusCode(StatusCode::UNAUTHORIZED);
    res.setBody("Login failed with invalid username or password");
    res.addHeader("Content-Length", std::to_string(res.getBody()->size()));
    res.addHeader("Content-Type", "text/plain");
    return res;
  }

  res.setStatusCode(StatusCode::FOUND);    // 302 Found
  res.addHeader("Location", "/dashboard"); // 跳转到用户主页
  res.setBody("Redirecting to /dashboard...");

  // 创建session
  // Set-Cookie: sessionid=abc123; Path=/; HttpOnly
  auto [sessionid, exireat] = session_manager_->create_session();
  cookie =
      "sessionid=" + sessionid + "; Path=" + valid_path_ + "; Expires=" + exireat + "; HttpOnly";
  res.addHeader("Set-Cookie", cookie);

  return res;
}

auto Authenticator::getInstance() -> Authenticator& {
  static Authenticator instance;
  return instance;
}

auto Authenticator::authenticate(const std::string& username, const std::string& password)
    -> bool { // NOLINT
  return username == "admin" && password == "admin";
}

auto Authenticator::logout(const std::string& session_id) -> void { // NOLINT
  (void)session_id;
}
