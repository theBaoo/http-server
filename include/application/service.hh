#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include <string>
#include <unordered_map>

#include "application/context.hh"
#include "application/file_service.hh"
#include "common/enum.hh"
#include "common/macro.hh"
#include "logging/logger.hh"

// Servce负责处理处理业务逻辑
// 但也应该负责针对Request Header来填写对应的Response Header
// 因为我不想在Parser-Service-Response之间再增加一个环节填写Header
// 构造和析构函数是否应该是public? 如果private, 谁负责析构?
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

 protected:
  // 直接使用private的话, 子类无法调用
  // 这是很常见的场景: 只希望派生类调用而不是外部类
  // 可以使用protected, 但其实这样的接口可以被抽出来复用, 使用组合来代替继承
  static auto innerError(ResponseContext& response, const std::string& msg) -> ResponseContext;
  static auto redirect(ResponseContext& response, const std::string& location) -> ResponseContext;
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

// 返回静态文件 html
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

    std::string path = request.getUri();
    if (path == "/") {
      path = "/index.html";
    } else if (path == "/dashboard") {
      path = "/dashboard.html";
    }
    std::string content = FileService::getInstance().getFileContent(path);

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