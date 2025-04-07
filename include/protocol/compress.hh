#ifndef PROTOCOL_COMPRESS_HH
#define PROTOCOL_COMPRESS_HH

#include <zlib.h>

#include <string>

#include "common/macro.hh"
#include "fmt/core.h"
#include "logging/logger.hh"

class Compressor {
 public:
  static auto compress(const std::string& raw) -> std::string;
  static auto decompress(const std::string& compressed) -> std::string;

  ENABLE_STATIC_INFO("compressor")
  ENABLE_STATIC_ERROR("compressor")
};

#endif // PROTOCOL_COMPRESS_HH