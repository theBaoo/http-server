#ifndef APPLICATION_IMG_SERVICE_H
#define APPLICATION_IMG_SERVICE_H

#include "application/service.hh"

class ImgService : public Service {
 public:
  static auto getInstance() -> ImgService& {
    static ImgService instance;
    return instance;
  }

  auto handle(RequestContext& request) -> ResponseContext override;
};

#endif // APPLICATION_IMG_SERVICE_H