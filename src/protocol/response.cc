#include "protocol/response.hh"
#include "protocol/compress.hh"

#include <string>

auto HTTPResponse::buildWithContext() const -> std::string {
  std::string response = "HTTP/1.1 " + std::to_string(static_cast<int>(ctx_.getStatusCode())) +
                         " " + ctx_.getStatusMessage() + CRLF;
  for (const auto& [field, value] : ctx_.getHeaders()) {
    response += field;
    response += ": ";
    response += value;
    response += CRLF;
  }
  response += CRLF;

  // 正文相关逻辑在Service中处理, 这里只负责简单拼接
  response += ctx_.getBody().value_or("");
  
  // 应该在哪里给ResponseContext添加Content-Encoding呢?
  if (ctx_.getHeader("Content-Encoding") == "gzip") {
    response = Compressor::compress(response);
  }
  return response;
}