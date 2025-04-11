#include "buffer/cache.hh"

#include <sys/stat.h>

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "common/constants.hh"
#include "logging/logger.hh"

// 基于文件名计算适用于改动很少的静态资源
auto getEtag(const std::string& uri) -> std::string {
  std::hash<std::string> hash_fn;
  auto                   hash = hash_fn(uri);
  std::string            etag = std::to_string(hash);
  return etag;
}

auto getLastModify(const std::string& uri) -> std::string {
  struct stat file_stat {};
  std::string path = stt + uri;
  // 这里承认了一个假设: 除了HTML文件无需后缀, 其他文件都需要后缀
  if (path.rfind(".html") == std::string::npos) {
    path += ".html";
  }
  Logger::getLogger("Cache::getLastModify").info("getLastModify: {}", path);
  if (stat(path.c_str(), &file_stat) == 0) {
    auto               last_modify = file_stat.st_mtime;
    std::tm*           tm_info = std::gmtime(&last_modify); // 使用 gmtime 转换为 UTC 时间
    std::ostringstream oss;
    oss << std::put_time(tm_info, "%a, %d %b %Y %H:%M:%S GMT"); // 格式化为 HTTP 日期格式
    return oss.str();
  }
  return "Unknown"; // 如果获取失败, 返回未知
}
