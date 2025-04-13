#include "application/route.hh"

#include "application/context.hh"
#include "buffer/cache.hh"
#include "protocol/compress.hh"

auto Router::forward(RequestContext& ctx) -> ResponseContext {
  ResponseContext result;

  if (ctx.getUri() == "/") {
    ctx.setUri("/index.html");
  }

  // TODO(thebao): 处理100
  // Expect: 100-continue
  // 则先发送100响应

  // 验证缓存
  if (ctx.getMethod() == "GET") {
    log("Checking cache for {}", ctx.getUri());
    auto time        = ctx.getHeader("If-Modified-Since");
    auto etag        = ctx.getHeader("If-None-Match");
    auto last_modify = getLastModify(ctx.getUri());
    auto etag_value  = getEtag(ctx.getUri());
    if ((!time.empty() && time == last_modify) || (!etag.empty() && etag == etag_value)) {
      log("Cache hit, returning 304 Not Modified");
      result.setStatusCode(StatusCode::NOT_MODIFIED);
      result.setStatusMessage("Not Modified");
      result.setBody("");
      return result;
    }
  }

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

  // 填写缓存头
  // TODO(thebao): GET和HEAD需要
  result.addHeader("Last-Modified", getLastModify(ctx.getUri()));
  result.addHeader("ETag", getEtag(ctx.getUri()));
  result.addHeader("Cache-Control", "public, max-age=3600");

  if (ctx.getMethod() == "HEAD") {
    result.setBody("");
  }

  result.addHeader("Agent", "BaoZongHuang");

  return result;
}