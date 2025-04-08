#include "application/route.hh"

#include "application/context.hh"
#include "protocol/compress.hh"

auto Router::forward(RequestContext& ctx) -> ResponseContext {
  ResponseContext result;
  if (ctx.getHeader("Content-Encoding") == "gzip") {
    ctx.setBody(Compressor::decompress(ctx.getBody()));
  }
  log("body: {}", ctx.getBody());

  // 拦截
  // 导致无限认证
  // result = (*getService(ctx.getUri() == "/dashboard" ? "/login" : ctx.getUri()))(ctx);

  if (ctx.getUri() == "/dashboard") {
    // NOT SET-COOKIE BUT COOKIE
    auto cookie = ctx.getHeader("Cookie");
    if (!cookie.empty() && cookie.find("sessionid=") != std::string::npos) {
      std::string sid     = cookie.substr(cookie.find('=') + 1);
      auto        session = SessionManager::getInstance().get_sesstion(sid);
      if (session.has_value()) {
        log("Authenticated successfully with session: {}", session->to_string());
        result = (*getService(ctx.getUri()))(ctx);
      } else {
        log("Session expired or invalid, redirecting to login page");
        result = (*getService("/login"))(ctx);
      }
    } else {
      log("Session not found, redirecting to login page");
      result = (*getService("/login"))(ctx);
    }
  } else {
    result = (*getService(ctx.getUri()))(ctx);
  }

  // 填写响应头
  if (ctx.getHeader("Accept-Encoding") == "gzip") {
    result.addHeader("Content-Encoding", "gzip");
  }

  if (ctx.getHeader("Connection") == "close") {
    result.addHeader("Connection", "close");
  }

  return result;
}