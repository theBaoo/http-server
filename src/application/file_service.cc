#include "application/file_service.hh"

#include <fstream>
#include <sstream>

#include "fmt/core.h"
#include "logging/logger.hh"

auto FileService::getFileContent(const std::string& uri) -> std::string {
  if (uri == "/") {
    return getFileContent("/index.html");
  }

  std::string path = root_ + uri;
  Logger::getLogger("file service").info("url: {}", path);
  std::ifstream file(path, std::ios::binary);
  if (file) {
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
  }
  return "";
}