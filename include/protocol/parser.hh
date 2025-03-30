#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <string>
#include <tuple>
#include <unordered_map>

#include "application/service.hh"
#include "common/macro.hh"
#include "fmt/color.h"
#include "logging/logger.hh"
#include "common/enum.hh"

// 解析 HTTP 请求行和头部字段
class Parser {
 public:
  using pair = std::pair<std::string, std::string>;

  static auto parse(std::string request) -> RequestContext;
  // Method URI Version 不允许多空格
  static auto parseRequest(std::string& request)
      -> std::tuple<std::string, std::string, std::string>;

  // Field: Value 只有value前允许多空格, 一般为可选空格(0-1)
  static auto parseHeader(std::string& header) -> std::tuple<std::string, std::string>;

  static auto parseBody(std::string& body, ContentType type)
      -> std::unordered_map<std::string, std::string>;

  static auto parseForm(std::string& body) -> pair;
  static auto parseJson(std::string& body) -> pair;
  static auto parsePlain(std::string& body) -> pair;
  static auto parseXml(std::string& body) -> pair;

  ENABLE_STATIC_INFO("parser")
  ENABLE_STATIC_ERROR("parser")
};

#endif // PROTOCOL_PARSER_H