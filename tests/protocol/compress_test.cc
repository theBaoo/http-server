#include <gtest/gtest.h>
#include <zlib.h>
#include <string>

#include "fmt/color.h"
#include "protocol/compress.hh"

TEST(CompressorTest, CompressTest) { // NOLINT
  std::string raw = "Hello, World!";
  std::string compressed = Compressor::compress(raw);
  fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::green), "Compressed: {}\n", compressed);
  ASSERT_FALSE(compressed.empty());
  ASSERT_NE(raw, compressed);
  ASSERT_LE(compressed.size(), compressBound(raw.size()));

}

TEST(CompressorTest, DecompressTest) { // NOLINT
  std::string raw = "Hello, World!";
  std::string compressed = Compressor::compress(raw);
  std::string decompressed = Compressor::decompress(compressed);
  ASSERT_EQ(raw, decompressed);
}