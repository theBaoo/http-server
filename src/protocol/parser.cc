#include "protocol/parser.hh"

#include <cstddef>
#include <iostream>
#include <string>
#include <tuple>

#include "application/service.hh"
#include "logging/logger.hh"
#include "protocol/uri_decoder.hh"

auto Parser::parse(std::string request) -> RequestContext {
  RequestContext ctx;

  auto [method, path, version] = parseRequest(request);
  auto replaced                = URIDecoder::replacePercent(path);
  auto uri                     = URIDecoder::decode(replaced);

  while (!request.empty()) {
    auto [field, value] = Parser::parseHeader(request);
    if (!field.empty() && !value.empty()) {
      ctx.addHeader(field, value);
    } else if (request.find_first_not_of("\r\n") == std::string::npos) {
      // 读取到空行，说明请求头部解析完毕
      break;
    }
  }

  ctx.setMethod(method);
  ctx.setUri(uri);
  ctx.setVersion(version);

  return ctx;
}

auto Parser::parseRequest(std::string& request)
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

auto Parser::parseHeader(std::string& header) -> std::tuple<std::string, std::string> {
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

auto Parser::parseBody(std::string& body, ContentType type)
    -> std::unordered_map<std::string, std::string> {
  auto parser = [type](std::string& body) -> std::pair<std::string, std::string> {
    switch (type) {
      case ContentType::FORM:
        return parseForm(body);
      case ContentType::JSON:
        return parseJson(body);
      case ContentType::PLAIN:
        return parsePlain(body);
      case ContentType::XML:
        return parseXml(body);
      default:
        return std::make_pair("", "");
    }
  };

  std::unordered_map<std::string, std::string> result;
  while (!body.empty()) {
    auto [field, value] = parser(body);
    if (!field.empty()) {
      result[field] = value;
    } else {
      error("Void field or value in body, field: {}, value: {}", field, value);
      break;
    }
  }
  return result;
}

auto Parser::parseForm(std::string& body) -> std::pair<std::string, std::string> {
  std::string field;
  std::string value;

  size_t pos = 0;
  pos        = body.find('=');
  if (pos != std::string::npos) {
    field = body.substr(0, pos);
    body.erase(0, pos + 1);
  } else {
    // 无法解析出 field, 这是不正常的情况
    error("Unable to resolve field in body {}", body);
    body.clear();
    return std::make_pair("", "");
  }

  pos = body.find('&');
  if (pos != std::string::npos) {
    value = body.substr(0, pos);
    body.erase(0, pos + 1);
  } else {
    value = body;
    body.clear();
  }

  return std::make_pair(field, value);
}

auto Parser::parseJson(std::string& body) -> std::pair<std::string, std::string> {
  // TODO(thebao): Implement JSON parser
  error("JSON parser is not implemented yet.");
  body.clear();
  return std::make_pair("", "");
}
auto Parser::parsePlain(std::string& body) -> std::pair<std::string, std::string> {
  // TODO(thebao): Implement plain parser
  error("Plain parser is not implemented yet.");
  body.clear();
  return std::make_pair("", "");
}
auto Parser::parseXml(std::string& body) -> std::pair<std::string, std::string> {
  // TODO(thebao): Implement XML parser
  error("XML parser is not implemented yet.");
  body.clear();
  return std::make_pair("", "");
}
