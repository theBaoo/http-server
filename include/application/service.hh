#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include <string>
#include <unordered_map>

#include "application/context.hh"
#include "application/file_service.hh"
#include "common/enum.hh"
#include "common/macro.hh"
#include "logging/logger.hh"

class Service {
 public:
  Service()          = default;
  virtual ~Service() = default;
  DISALLOW_COPY_AND_MOVE(Service);

  static auto getInstance() -> Service&;

  auto operator()(RequestContext& request) -> ResponseContext {
    return handle(request);
  }

  virtual auto handle(RequestContext&) -> ResponseContext = 0;
};

class UnimplementedService : public Service {
 public:
  UnimplementedService();
  ~UnimplementedService() override = default;
  DISALLOW_COPY_AND_MOVE(UnimplementedService);
  // TODO(thebao): 解决参数容易混淆的问题
  // Fixed: 抽象类传递数据
  auto handle(RequestContext& request) -> ResponseContext override;

 private:
  ResponseContext ctx;
};

// 总是返回根目录下的index.html
class DefaultService : public Service {
 public:
  DefaultService();
  ~DefaultService() override = default;
  DISALLOW_COPY_AND_MOVE(DefaultService);

  static auto getInstance() -> DefaultService& {
    static DefaultService instance;
    return instance;
  }

  auto handle([[maybe_unused]] RequestContext& request) -> ResponseContext override {
    ResponseContext result;
    Logger::getLogger("default service").info("Default service is called");

    std::string content = FileService::getInstance().getFileContent("/index.html");

    result.setStatusCode(StatusCode::OK);
    result.setStatusMessage("OK");
    result.setBody(content);
    result.addHeader("Content-Type", "text/html");
    // 添加body的长度, 否则浏览器可能停止渲染
    result.addHeader("Content-Length", std::to_string(content.size()));

    return result;
  }

 private:
  ResponseContext ctx;
};

#endif // APPLICATION_SERVICE_H