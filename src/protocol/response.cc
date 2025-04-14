#include "protocol/response.hh"

#include <string>

#include "protocol/compress.hh"

auto HTTPResponse::buildWithContext() -> std::string {
  std::string response = "HTTP/1.1 " + std::to_string(static_cast<int>(ctx_.getStatusCode())) +
                         " " + ctx_.getStatusMessage() + CRLF;

  // 正文相关逻辑在Service中处理, 这里只负责简单拼接
  auto body = ctx_.getBody().value_or("");
  if (ctx_.getHeader("Content-Encoding") == "gzip") {
    body     = Compressor::compress(body);
    auto len = std::to_string(body.size());
    ctx_.addHeader("Content-Length", len);
  }

  for (const auto& [field, value] : ctx_.getHeaders()) {
    response += field;
    response += ": ";
    response += value;
    response += CRLF;
  }
  response += CRLF;

  response += body;

  return response;
}