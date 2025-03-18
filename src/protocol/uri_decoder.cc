#include "protocol/uri_decoder.hh"

auto URIDecoder::decode(const std::string& uri) -> std::string {
  std::string decoded;
  for (size_t i = 0; i < uri.size(); ++i) {
    if (uri[i] == '%') {
      if (i + 2 < uri.size()) {
        int value = std::stoi(uri.substr(i + 1, 2), nullptr, HEX);
        decoded += static_cast<char>(value);
        i += 2;
      }
    } else {
      decoded += uri[i];
    }
  }
  return decoded;
}