#include "application/service.hh"

#include "protocol/response.hh"

UnimplementedService::UnimplementedService() {
  ctx.setStatusCode(StatusCode::NOT_IMPLEMENTED);
  ctx.setStatusMessage("This service is not implemented yet.");
  ctx.setBody("Sorry for unimplemented service.");
  ctx.addHeader("Content-Type", "text/html");
}

auto UnimplementedService::handle([[maybe_unused]] RequestContext &request) -> ResponseContext {
  return ctx;
}

DefaultService::DefaultService() {
}