#ifndef APPLICATION_CONTROL_SERVICE_HH
#define APPLICATION_CONTROL_SERVICE_HH

#include "application/login_service.hh"
#include "application/service.hh"

class ControlService : public Service {
 public:
  ControlService();
  ~ControlService() override = default;
  DISALLOW_COPY_AND_MOVE(ControlService);

  static auto getInstance() -> ControlService&;
  auto        handle(RequestContext& ctx) -> ResponseContext override;

 private:
  Authenticator* authenticator_;
};

#endif // APPLICATION_CONTROL_SERVICE_HH