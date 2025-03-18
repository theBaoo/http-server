#include "protocol/parser.hh"

#include <iostream>
#include <string>
#include <tuple>

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
  }
  return std::make_tuple(method, path, version);
}

auto Parser::parserHeader(std::string &header) -> std::tuple<std::string, std::string> {
  std::string field;
  std::string value;

  size_t pos = 0;
  pos        = header.find(':');
  if (pos != std::string::npos) {
    field = header.substr(0, pos);
    header.erase(0, pos + 1);
  }
  pos = header.find("\r\n");
  if (pos != std::string::npos) {
    value = header.substr(0, pos);
  }
  return std::make_tuple(field, value);
}