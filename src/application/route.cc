#include "application/route.hh"

#include "protocol/compress.hh"

auto Router::forward(RequestContext ctx) -> ResponseContext {
  if (ctx.getHeader("Content-Encoding") == "gzip") {
    ctx.setBody(Compressor::decompress(ctx.getBody()));
  }
  log("body: {}", ctx.getBody());
  auto result = (*services_[ctx.getUri()])(ctx);

  // 填写响应头
  if (ctx.getHeader("Accept-Encoding") == "gzip") {
    result.addHeader("Content-Encoding", "gzip");
  }

  return result;
}