#include "protocol/compress.hh"

#include <zconf.h>
#include <zlib.h>

#include <iostream> // 添加日志输出
#include <string>
#include <vector>
#include <array> // Required for std::array

#include "logging/logger.hh"

auto Compressor::compress(const std::string &raw) -> std::string {
  z_stream stream{};
  int      ret =
      deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
  if (ret != Z_OK) {
    error("deflateInit failed with error code: {}", ret);
    throw std::runtime_error("Failed to initialize zlib");
  }

  stream.next_in  = reinterpret_cast<Bytef *>(const_cast<char *>(raw.data())); // NOLINT
  stream.avail_in = static_cast<uInt>(raw.size());

  constexpr size_t              chunk_size = 16384; // 16KB 块，经验值
  std::vector<char>             out_buffer;
  std::array<Bytef, chunk_size> temp{};

  do {
    stream.next_out  = temp.data();
    stream.avail_out = static_cast<uInt>(temp.size());

    ret = deflate(&stream, Z_FINISH);

    if (ret != Z_OK && ret != Z_STREAM_END) {
      deflateEnd(&stream);
      throw std::runtime_error("deflate failed during compression");
    }

    size_t bytes_written = temp.size() - stream.avail_out;
    out_buffer.insert(out_buffer.end(), temp.data(), temp.data() + bytes_written);
  } while (ret != Z_STREAM_END);

  deflateEnd(&stream);

  return { out_buffer.begin(), out_buffer.end() };
}

auto Compressor::decompress(const std::string &compressed) -> std::string {
  z_stream stream{};
  // 使用 inflateInit2 以支持 gzip 和 zlib 格式
  // 如使用 inflateInit，可能会导致 gzip 格式的文件无法解压
  int ret = inflateInit2(&stream, 15 + 32); // 15 是默认窗口大小，+32 允许自动检测 gzip/zlib 格式
  if (ret != Z_OK) {
    error("inflateInit2 failed with error code: {} and msg: {}", ret, stream.msg);
    throw std::runtime_error("Failed to initialize zlib");
  }

  stream.next_in  = reinterpret_cast<Bytef *>(const_cast<char *>(compressed.data())); // NOLINT
  stream.avail_in = static_cast<uInt>(compressed.size());

  std::vector<Bytef> buffer(compressed.size() * 2); // Initial buffer size
  std::string        decompressed_data;

  // std::cerr << "Compressed data (first 16 bytes): ";
  // for (size_t i = 0; i < std::min<size_t>(compressed.size(), 16); ++i) {
  //   std::cerr << std::hex << static_cast<int>(compressed[i] & 0xFF) << " ";
  // }
  std::cerr << std::dec << std::endl;

  do {
    stream.next_out  = buffer.data();
    stream.avail_out = static_cast<uInt>(buffer.size());

    ret = inflate(&stream, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      error("inflate failed with error code: {} and msg: {}", ret, stream.msg);
      inflateEnd(&stream);
      throw std::runtime_error("Failed to decompress data");
    }

    decompressed_data.append(reinterpret_cast<char *>(buffer.data()),
                             buffer.size() - stream.avail_out);
    log("Decompression progress: {}", decompressed_data.size());
  } while (ret != Z_STREAM_END);

  log("Decompression successful, decompressed size: {}", decompressed_data.size());

  inflateEnd(&stream);

  return decompressed_data;
}