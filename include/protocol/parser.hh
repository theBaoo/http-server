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
  // Method URI Version 不允许多空格
  static auto parseRequest(std::string& request)
      -> std::tuple<std::string, std::string, std::string>;

  // Field: Value 只有value前允许多空格, 一般为可选空格(0-1)
  static auto parseHeader(std::string& header) -> std::tuple<std::string, std::string>;
};

#endif // PROTOCOL_PARSER_H