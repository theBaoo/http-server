#include "application/route.hh"

auto Router::forward(RequestContext ctx) -> ResponseContext {
  log("body: {}", ctx.getBody());
  return (*services_[ctx.getUri()])(ctx);
}
