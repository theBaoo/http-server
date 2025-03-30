#ifndef PROTOCOL_RESPONSE_H
#define PROTOCOL_RESPONSE_H

#include <map>
#include <string>
#include "application/service.hh"
#include "common/enum.hh"
#include "application/context.hh"

const std::string HTTPV = "HTTP/1.1";
const std::string CRLF  = "\r\n";

// constexpr std::string_view getReasonPhrase(HttpStatus status) {
//   switch (status) {
//     case HttpStatus::OK:
//       return "OK";
//     case HttpStatus::NotFound:
//       return "Not Found";
//     case HttpStatus::InternalServerError:
//       return "Internal Server Error";
//     default:
//       return "Unknown";
//   }
// }

class HTTPResponse {
 public:
  // 构造时指定请求方法，影响响应行为
  explicit HTTPResponse(std::string method = "GET")
      : method_(std::move(method)), status_message_("OK") {
    headers_["Server"] = "SimpleHTTPServer"; // 默认头部
  }

  explicit HTTPResponse(ResponseContext& ctx) : ctx_(ctx) {
    headers_["Server"] = "SimpleHTTPServer"; // 默认头部
  }

  // 设置状态
  auto setStatus(StatusCode code, const std::string& message = "") -> HTTPResponse& {
    status_code_    = code;
    status_message_ = message.empty() ? defaultMessage(code) : message;
    return *this;
  }

  // 添加头部
  auto addHeader(const std::string& field, const std::string& value) -> HTTPResponse& {
    headers_[field] = value;
    return *this;
  }

  // 设置正文（可选）
  auto setBody(const std::string& body) -> HTTPResponse& {
    body_ = body;
    if (method_ != "HEAD" && body_.has_value()) {
      headers_["Content-Length"] = std::to_string(body_->size());
    }
    return *this;
  }

  // 根据请求方法和上下文构建响应字符串
  [[nodiscard]] auto build() const -> std::string {
    validate();

    std::string response =
        "HTTP/1.1 " + std::to_string(static_cast<int>(status_code_)) + " " + status_message_ + CRLF;
    for (const auto& [field, value] : headers_) {
      response += field;
      response += ": ";
      response += value;
      response += CRLF;
    }
    response += CRLF;

    // HEAD 方法不返回正文
    if (method_ != "HEAD" && body_.has_value()) {
      response += *body_;
    }
    return response;
  }

  [[nodiscard]] auto buildWithContext() const -> std::string;

  // 获取当前方法（用于调试或外部逻辑）
  [[nodiscard]] auto getMethod() const -> std::string {
    return method_;
  }

 private:
  static auto defaultMessage(StatusCode code) -> std::string {
    switch (code) {
      case StatusCode::OK:
        return "OK";
      // case StatusCode::CREATED:
      //   return "Created";
      case StatusCode::BAD_REQUEST:
        return "Bad Request";
      case StatusCode::NOT_FOUND:
        return "Not Found";
      // case StatusCode::METHOD_NOT_ALLOWED:
      //   return "Method Not Allowed";
      default:
        return "Unknown";
    }
  }

  void validate() const {
    if (method_ != "GET" && method_ != "POST" && method_ != "HEAD") {
      throw std::runtime_error("Unsupported HTTP method: " + method_);
    }
    if (status_code_ == StatusCode::OK && !body_.has_value() && method_ != "HEAD") {
      throw std::runtime_error("Body required for OK status in " + method_);
    }
  }

  std::string                        method_; // 请求方法，影响响应行为
  StatusCode                         status_code_ = StatusCode::OK;
  std::string                        status_message_;
  std::map<std::string, std::string> headers_;
  std::optional<std::string>         body_; // 使用 optional 支持无正文情况
  ResponseContext ctx_;
};

#endif // PROTOCOL_RESPONSE_H