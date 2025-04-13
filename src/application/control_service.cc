#include "application/control_service.hh"

#include "application/context.hh"
#include "application/upload_service.hh"
#include "common/enum.hh"

extern volatile sig_atomic_t g_shutdown; // NOLINT

ControlService::ControlService() : authenticator_(&Authenticator::getInstance()) {
}

auto ControlService::getInstance() -> ControlService& {
  static ControlService instance;
  return instance;
}
auto ControlService::handle(RequestContext& ctx) -> ResponseContext {
  ResponseContext response;
  if (ctx.getMethod() != "POST") {
    return unAuthorized(response, "Method Not Allowed!");
  }

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

  auto valid = authenticator_->authenticate(username, password);
  if (!valid) {
    return unAuthorized(response, "Invalid username or password!");
  }

  if (ctx.getUri().rfind("stop") != std::string::npos) {
    response.setStatusCode(StatusCode::OK);
    response.setStatusMessage("OK");
    response.setBody("App is going to shuntdown!");
    response.addHeader("Content-Type", "text/plain");
    response.addHeader("Content-Length", std::to_string(response.getBody()->size()));

    g_shutdown = 1;
  }

  return response;
}