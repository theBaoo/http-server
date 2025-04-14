#ifndef APPLICATION_UPLOAD_SERVICE_HH
#define APPLICATION_UPLOAD_SERVICE_HH

#include <string>

#include "application/service.hh"
#include "common/macro.hh"

class UploadService : public Service {
 public:
  UploadService();
  ~UploadService() override = default;
  DISALLOW_COPY_AND_MOVE(UploadService);

  static auto getInstance() -> UploadService&;
  auto        handle(RequestContext& ctx) -> ResponseContext override;

 private:
  std::string valid_path_ = "/upload";

  ENABLE_INFO("UploadService")
  ENABLE_ERROR("UploadService")
};

struct FormInfo {
  std::string                name;
  std::optional<std::string> filename;
  std::string                content_type;
  std::string                content;
};

auto saveFile(const std::string& filename, const std::string& content) -> bool;

auto parseMultipartFormData(const std::string_view& body, const std::string& boundary)
    -> std::vector<FormInfo>;

#endif // APPLICATION_UPLOAD_SERVICE_HH