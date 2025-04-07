#ifndef PROTOCOL_COMPRESS_HH
#define PROTOCOL_COMPRESS_HH

#include <zlib.h>
#include <string>
#include "common/macro.hh"
#include "logging/logger.hh"
#include "fmt/core.h"

class Compressor {
 public:
  static auto compress(const std::string& raw) -> std::string;
  static auto decompress(const std::string& compressed) -> std::string;

  ENABLE_STATIC_INFO("compressor")
  ENABLE_STATIC_ERROR("compressor")
};

#endif // PROTOCOL_COMPRESS_HH