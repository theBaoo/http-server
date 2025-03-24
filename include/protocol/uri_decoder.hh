#ifndef PROTOCOL_URI_DECODER_H
#define PROTOCOL_URI_DECODER_H

#include <string>
#include <unordered_map>

const int HEX = 16;

class URIDecoder {
 public:
  // /path/to/file
  static auto decode(const std::string& uri) -> std::string;
  // ?field=value&
  static auto parseParam(const std::string& uri) -> std::unordered_map<std::string, std::string>;

  static auto replacePercent(const std::string& uri) -> std::string;
};

#endif // PROTOCOL_URI_DECODER_H