extern "C" {
#include "dbus.h"
}  // extern "C"

#include <sdbus-c++/AdaptorInterfaces.h>
#include <sdbus-c++/IConnection.h>

#include <atomic>
#include <csignal>
#include <memory>
#include <string>
#include <utility>

#include "wlsunset-server-glue.h"

namespace {
class wlsunset final
    : public sdbus::AdaptorInterfaces<org::jjaro::wlsunset_adaptor> {
 public:
  wlsunset(sdbus::IConnection &connection, std::string objectPath)
      : AdaptorInterfaces(connection, std::move(objectPath)), inhibit_{false} {
    registerAdaptor();
  }
  ~wlsunset() { unregisterAdaptor(); }
  bool inhibit() const { return inhibit_.load(std::memory_order_relaxed); }

 private:
  bool get_inhibit() override {
    return inhibit_.load(std::memory_order_relaxed);
  }
  void set_inhibit(const bool &inhibit) override {
    const bool old = inhibit_.exchange(inhibit, std::memory_order_relaxed);
    if (old == inhibit) return;
    emitWatch_inhibit(inhibit);
    raise(SIGALRM);
  }
  bool toggle_inhibit() override {
    bool guess = false;
    while (!inhibit_.compare_exchange_weak(guess, !guess,
                                           std::memory_order_relaxed));
    emitWatch_inhibit(!guess);
    raise(SIGALRM);
    return !guess;
  }

  std::atomic<bool> inhibit_;
};

struct state {
  std::unique_ptr<sdbus::IConnection> c;
  std::unique_ptr<wlsunset> s;
};
}  // namespace

extern "C" {
void *StartWlsunsetDbus(void) {
  auto s = new state;
  s->c = sdbus::createSessionBusConnection("org.jjaro.wlsunset");
  s->s = std::make_unique<wlsunset>(*s->c, "/org/jjaro/wlsunset");
  s->c->enterEventLoopAsync();
  return s;
}

void StopWlsunsetDbus(void *v) {
  state *s = reinterpret_cast<state *>(v);
  s->c->leaveEventLoop();
  delete s;
}

int IsInhibited(const void *v) {
  const state *s = reinterpret_cast<const state *>(v);
  return s->s->inhibit();
}
}  // extern "C"
