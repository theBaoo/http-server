#ifndef APPLICATION_ROUTE_H
#define APPLICATION_ROUTE_H

#include <future>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "application/file_service.hh"
#include "application/img_service.hh"
#include "application/service.hh"

class Service;

class Router {
  using RouterTable = std::unordered_map<std::string, Service*>;

 public:
  auto registerService(const std::string& uri, Service* service) -> void {
    services_[uri] = service;
  }

  // 默认构造的Service调用handle会导致address boundry error
  auto getService(const std::string& uri) -> Service* {
    if (services_.find(uri) == services_.end()) {
      // throw std::runtime_error("Service not found");
      static UnimplementedService unimplementedService;
      return &unimplementedService;
    }
    return services_[uri];
  }

  auto operator[](const std::string& uri) -> Service* {
    return getService(uri);
  }

 private:
  // Router 只负责动态路由, 不负责管理Service的生命周期, 所以直接只用raw pointer
  // Service的生命周期由自身管理
  // 使用raw pointer出于两种考虑, 更灵活
  // 1. Service可以为空
  // 2. 可以动态更换Service, 而引用不行
  RouterTable services_;
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
      router.registerService("/", &DefaultService::getInstance());
      router.registerService("/index.html", &DefaultService::getInstance());
      router.registerService("/favicon.ico", &ImgService::getInstance());
    }
    return router;
  }
};

#endif // APPLICATION_ROUTE_H