#include "application/img_service.hh"

#include "application/file_service.hh"
#include "protocol/response.hh"

auto ImgService::handle([[maybe_unused]] RequestContext& request) -> ResponseContext {
  ResponseContext response;
  std::string     img = FileService::getInstance().getFileContent("/favicon.ico");
  response.setStatusCode(StatusCode::OK);
  response.setStatusMessage("OK");
  response.setBody(img);
  response.addHeader("Content-Type", "image/x-icon");
  return response;
}