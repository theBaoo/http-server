#ifndef APPLICATION_FILE_SERVICE_H
#define APPLICATION_FILE_SERVICE_H

#include <string>

#include "common/constants.hh"
#include "common/macro.hh"

// 将请求路径映射到static目录
// 单例
class FileService {
 public:
  // Thread-safe?
  auto static getInstance() -> FileService& {
    static FileService instance;
    return instance;
  }

  void setRoot(const std::string& root) {
    root_ = root;
  }

  [[nodiscard]] auto getRoot() const -> std::string {
    return root_;
  }

  auto getFileContent(const std::string& uri) -> std::string;

 private:
  FileService() : root_(root + "/static"){};
  DISALLOW_COPY_AND_MOVE(FileService);
  ~FileService() = default;

  std::string root_;
};
#endif // APPLICATION_FILE_SERVICE_H