#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include <string>
#include <unordered_map>

#include "application/file_service.hh"
#include "common/macro.hh"
#include "logging/logger.hh"
#include "protocol/parser.hh"
#include "protocol/response.hh"

class RequestContext {
  using Headers = std::unordered_map<std::string, std::string>;
  using Params  = std::unordered_map<std::string, std::string>;

 private:
  std::string uri_;
  std::string method_;
  std::string version_;
  Headers     headers_;
  Params      params_;

 public:
  DEFINE_GETTER_AND_SETTER(std::string, Uri, uri_);
  DEFINE_GETTER_AND_SETTER(std::string, Method, method_);
  DEFINE_GETTER_AND_SETTER(std::string, Version, version_);

  auto addHeader(const std::string& field, const std::string& value) -> RequestContext& {
    headers_[field] = value;
    return *this;
  }

  [[nodiscard]] auto getHeader(const std::string& field) const -> std::string {
    if (headers_.find(field) != headers_.end()) {
      return headers_.at(field);
    }
    return "";
  }

  auto addParam(const std::string& field, const std::string& value) -> RequestContext& {
    params_[field] = value;
    return *this;
  }

  [[nodiscard]] auto getParam(const std::string& field) const -> std::string {
    if (params_.find(field) != params_.end()) {
      return params_.at(field);
    }
    return "";
  }
};

class ResponseContext {
  using Headers = std::unordered_map<std::string, std::string>;

 private:
  StatusCode                 status_code_{};
  std::string                status_message_;
  Headers                    headers_;
  std::optional<std::string> body_;

 public:
  DEFINE_GETTER_AND_SETTER(StatusCode, StatusCode, status_code_)
  DEFINE_GETTER_AND_SETTER(std::string, StatusMessage, status_message_)
  DEFINE_GETTER_AND_SETTER(std::optional<std::string>, Body, body_)

  auto addHeader(const std::string& field, const std::string& value) -> ResponseContext& {
    headers_[field] = value;
    return *this;
  }

  [[nodiscard]] auto getHeader(const std::string& field) const -> std::string {
    if (headers_.find(field) != headers_.end()) {
      return headers_.at(field);
    }
    return "";
  }
};

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
  UnimplementedService()           = default;
  ~UnimplementedService() override = default;
  DISALLOW_COPY_AND_MOVE(UnimplementedService);
  // TODO(thebao): 解决参数容易混淆的问题
  // Fixed: 抽象类传递数据
  auto handle(RequestContext& request) -> ResponseContext override {
    (void)request;
    static ResponseContext result;
    return result;
  }
};

// 总是返回根目录下的index.html
class DefaultService : public Service {
 public:
  DefaultService()           = default;
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

    return result;
  }
};

#endif // APPLICATION_SERVICE_H