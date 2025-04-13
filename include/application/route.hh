#ifndef APPLICATION_ROUTE_H
#define APPLICATION_ROUTE_H

#include <future>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

// TODO(thebao): 优化include, 其实只需要提供服务类的声明. 考虑在哪里统一注册
#include "application/cgi_service.hh"
#include "application/control_service.hh"
#include "application/file_service.hh"
#include "application/img_service.hh"
#include "application/login_service.hh"
#include "application/service.hh"
#include "application/upload_service.hh"
#include "buffer/session.hh"
#include "common/macro.hh"

class Service;

// TODO(thebao): 路由树, 分段匹配路径
class Router {
  using RouterTable   = std::unordered_map<std::string, Service*>;
  using PreRouteTable = std::unordered_map<std::string, Service*>;

 public:
  auto forward(RequestContext& ctx) -> ResponseContext;

  auto registerService(const std::string& uri, Service* service) -> Router& {
    services_[uri] = service;
    return *this;
  }

  auto registerPreService(const std::string& uri, Service* service) -> Router& {
    preServices_[uri] = service;
    return *this;
  }

  // 默认构造的Service调用handle会导致address boundry error
  auto getService(const std::string& uri) -> Service* {
    auto itr = services_.find(uri);
    if (itr != services_.end()) {
      return itr->second;
    }

    for (const auto& [pre, service] : preServices_) {
      log("pre: {}, uri: {}", pre, uri);
      if (uri.rfind(pre, 0) == 0) {
        return service;
      }
    }

    static UnimplementedService unimplementedService;
    return &unimplementedService;
  }

  auto operator[](const std::string& uri) -> Service* {
    return getService(uri);
  }

  ENABLE_INFO("router")
  ENABLE_ERROR("router")

 private:
  // Router 只负责动态路由, 不负责管理Service的生命周期, 所以直接只用raw pointer
  // Service的生命周期由自身管理
  // 使用raw pointer出于两种考虑, 更灵活
  // 1. Service可以为空
  // 2. 可以动态更换Service, 而引用不行
  RouterTable   services_;
  PreRouteTable preServices_;

  SessionManager* session_manager_ = &SessionManager::getInstance();
};

// 1. Route做单例是否可行
// 2. 做动态路由需要FileService的默认构造函数? 但出于单例设计这是private的. 哪里出了问题?

class ServiceFactory {
 public:
  static auto createRouter() -> Router& {
    static Router router;
    static bool   initialized = false;
    if (!initialized) {
      initialized = true;
      // 精确路由
      router.registerService("/", &DefaultService::getInstance())
          .registerService("/index.html", &DefaultService::getInstance())
          .registerService("/favicon.ico", &ImgService::getInstance())
          .registerService("/login", &LoginService::getInstance())
          .registerService("/dashboard", &DefaultService::getInstance())
          .registerService("/cache-test", &DefaultService::getInstance())
          .registerService("/upload", &UploadService::getInstance())
          // 前缀路由
          .registerPreService("/cgi-bin/", &CgiService::getInstance())
          .registerPreService("/ctrl/", &ControlService::getInstance());
    }
    return router;
  }
};

#endif // APPLICATION_ROUTE_H