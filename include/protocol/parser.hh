#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <string>
#include <tuple>

enum class HTTPMethod {
  GET,
  POST,
  PUT,
  DELETE,
  HEAD,
  OPTIONS,
  TRACE,
  CONNECT,
};

// 解析 HTTP 请求行和头部字段
class Parser {
 public:
  // Method, Path, Version
  static auto parseRequest(std::string& request)
      -> std::tuple<std::string, std::string, std::string>;

  // Field, Value
  static auto parserHeader(std::string& header) -> std::tuple<std::string, std::string>;
};

#endif // PROTOCOL_PARSER_H