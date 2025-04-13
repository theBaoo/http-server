#include "application/service.hh"

#include "protocol/response.hh"

auto Service::innerError(ResponseContext &response, const std::string &msg) -> ResponseContext {
  response.setStatusCode(StatusCode::INTERNAL_SERVER_ERROR);
  response.setStatusMessage("Internal Server Error: " + msg);
  response.setBody("Internal Server Error: " + msg);
  return response;
}

auto Service::redirect(ResponseContext &response, const std::string &location) -> ResponseContext {
  response.setStatusCode(StatusCode::FOUND);
  response.setStatusMessage("Found");
  response.addHeader("Location", location);
  response.setBody("Redirecting to " + location);
  return response;
}

auto Service::unAuthorized(ResponseContext &response, const std::string &msg) -> ResponseContext {
  response.setStatusCode(StatusCode::UNAUTHORIZED);
  response.setStatusMessage("Unauthorized: " + msg);
  response.setBody("Unauthorized: " + msg);
  return response;
}

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