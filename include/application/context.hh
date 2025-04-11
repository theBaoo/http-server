#ifndef APPLICATION_CONTEXT_HH
#define APPLICATION_CONTEXT_HH

#include <optional>
#include <string>
#include <unordered_map>

#include "common/enum.hh"
#include "common/macro.hh"

class RequestContext {
  using Headers = std::unordered_map<std::string, std::string>;
  using Params  = std::unordered_map<std::string, std::string>;

 private:
  std::string uri_;
  std::string method_;
  std::string version_;
  std::string body_;
  Headers     headers_;
  Params      params_;

 public:
  DEFINE_GETTER_AND_SETTER(std::string, Uri, uri_);
  DEFINE_GETTER_AND_SETTER(std::string, Method, method_);
  DEFINE_GETTER_AND_SETTER(std::string, Version, version_);

  [[nodiscard]] auto getBody() const -> std::string {
    return body_;
  }

  auto setBody(const std::string& body) -> RequestContext& {
    body_ = body;
    return *this;
  }

  auto addBody(const std::string& body) -> RequestContext& {
    body_ += body;
    return *this;
  }

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

  [[nodiscard]] auto getHeaders() const -> Headers {
    return headers_;
  }
};

#endif // APPLICATION_CONTEXT_HH