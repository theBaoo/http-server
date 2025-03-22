#include "protocol/parser.hh"

#include <iostream>
#include <string>
#include <tuple>

#include "logging/logger.hh"

auto Parser::parseRequest(std::string &request)
    -> std::tuple<std::string, std::string, std::string> {
  std::string method;
  std::string path;
  std::string version;

  size_t pos = 0;
  pos        = request.find(' ');
  if (pos != std::string::npos) {
    method = request.substr(0, pos);
    request.erase(0, pos + 1);
  }
  pos = request.find(' ');
  if (pos != std::string::npos) {
    path = request.substr(0, pos);
    request.erase(0, pos + 1);
  }
  pos = request.find("\r\n");
  if (pos != std::string::npos) {
    version = request.substr(0, pos);
    request.erase(0, pos + 2);
  }
  return std::make_tuple(method, path, version);
}

auto Parser::parseHeader(std::string &header) -> std::tuple<std::string, std::string> {
  std::string field;
  std::string value;

  size_t pos = 0;
  pos        = header.find(':');
  if (pos != std::string::npos) {
    field = header.substr(0, pos);
    header.erase(0, pos + 1);
  } else {
    // 无法解析出 field, 这是不正常的情况
    header.clear();
    return std::make_tuple("", "");
  }

  // 跳过 : 后的空格
  while (header[0] == ' ') {
    header.erase(0, 1);
  }

  pos = header.find("\r\n");
  if (pos != std::string::npos) {
    value = header.substr(0, pos);
    header.erase(0, pos + 2);
  } else {
    value = header;
    header.clear();
  }
  return std::make_tuple(field, value);
}