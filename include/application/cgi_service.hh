#ifndef APPLICATION_CGI_SERVICE_H
#define APPLICATION_CGI_SERVICE_H

#include "application/context.hh"
#include "application/service.hh"
#include "common/macro.hh"

class CgiService : public Service {
 public:
  CgiService() = default;
  ~CgiService() override = default;

  DISALLOW_COPY_AND_MOVE(CgiService);

  static auto getInstance() -> CgiService& {
    static CgiService instance;
    return instance;
  }

  // TODO(thebao): 响应的请求头也应该从cgi脚本中获取, 考虑使用Parser, 但是对输出有要求, "\r\n"
  auto handle(RequestContext& ctx) -> ResponseContext override;

  ENABLE_INFO("cgi service")
  ENABLE_ERROR("cgi service")
};

#endif // APPLICATION_CGI_SERVICE_H