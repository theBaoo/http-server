#ifndef PROTOCOL_URI_DECODER_H
#define PROTOCOL_URI_DECODER_H

#include <string>

const int HEX = 16;

class URIDecoder {
 public:
  static auto decode(const std::string& uri) -> std::string;
};

#endif // PROTOCOL_URI_DECODER_H