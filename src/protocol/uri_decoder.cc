#include "protocol/uri_decoder.hh"
#include <cstddef>

auto URIDecoder::decode(const std::string& uri) -> std::string {
  std::string decoded = uri;
  size_t pos = decoded.find('?');
  if (pos != std::string::npos) {
    decoded = uri.substr(0, pos);
  }
  return decoded;
}

auto URIDecoder::parseParam(const std::string& uri) -> std::unordered_map<std::string, std::string> {
  std::unordered_map<std::string, std::string> params;
  size_t pos = uri.find('?');
  if (pos == std::string::npos) { return params; }
  
  std::string query = uri.substr(pos + 1);
  size_t      start = 0;
  while (start < query.size()) {
    size_t end = query.find('&', start);
    if (end == std::string::npos) {
      end = query.size();
    }
    size_t equal = query.find('=', start);
    if (equal != std::string::npos) {
      params[query.substr(start, equal - start)] = query.substr(equal + 1, end - equal - 1);
    }
    start = end + 1;
  }
  
  return params;
}

auto URIDecoder::replacePercent(const std::string& uri) -> std::string {
  std::string replaced;
  for (size_t i = 0; i < uri.size(); ++i) {
    if (uri[i] == '%') {
      if (i + 2 < uri.size()) {
        int value = std::stoi(uri.substr(i + 1, 2), nullptr, HEX);
        replaced += static_cast<char>(value);
        i += 2;
      }
    } else {
      replaced += uri[i];
    }
  }
 
  return replaced;
}