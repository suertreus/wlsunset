#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/ProxyInterfaces.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>

#include "absl/functional/any_invocable.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "wlsunset-client-glue.h"

namespace {
class wlsunsetProxy final
    : public sdbus::ProxyInterfaces<org::jjaro::wlsunset_proxy> {
 public:
  wlsunsetProxy(
      sdbus::IConnection& connection, std::string destination,
      std::string objectPath,
      absl::AnyInvocable<void(bool)> watch_inhibit = [](bool) {})
      : ProxyInterfaces(connection, std::move(destination),
                        std::move(objectPath)),
        watch_inhibit_(std::move(watch_inhibit)) {
    registerProxy();
  }

  ~wlsunsetProxy() { unregisterProxy(); }

 private:
  void onWatch_inhibit(const bool& inhibit) override {
    watch_inhibit_(inhibit);
  }

  absl::AnyInvocable<void(bool)> watch_inhibit_;
};
}  // namespace

int main(int argc, char** argv) {
  if (argc == 2 && argv && argv[1] &&
      absl::string_view(argv[1]) == "get_inhibit") {
    auto connection = sdbus::createSessionBusConnection();
    absl::PrintF("%d\n", wlsunsetProxy(*connection, "org.jjaro.wlsunset",
                                       "/org/jjaro/wlsunset")
                                 .get_inhibit()
                             ? 1
                             : 0);
    return EXIT_SUCCESS;
  } else if (argc == 3 && argv && argv[1] &&
             absl::string_view(argv[1]) == "set_inhibit") {
    bool arg;
    if (argv[2] && absl::SimpleAtob(argv[2], &arg)) {
      auto connection = sdbus::createSessionBusConnection();
      wlsunsetProxy(*connection, "org.jjaro.wlsunset", "/org/jjaro/wlsunset")
          .set_inhibit(arg ? true : false);
      return EXIT_SUCCESS;
    }
  } else if (argc == 2 && argv && argv[1] &&
             absl::string_view(argv[1]) == "toggle_inhibit") {
    auto connection = sdbus::createSessionBusConnection();
    absl::PrintF("%d\n", wlsunsetProxy(*connection, "org.jjaro.wlsunset",
                                       "/org/jjaro/wlsunset")
                                 .toggle_inhibit()
                             ? 1
                             : 0);
    return EXIT_SUCCESS;
  } else if (argc == 2 && argv && argv[1] &&
             absl::string_view(argv[1]) == "watch_inhibit") {
    (void)setvbuf(stdout, nullptr, _IOLBF, 0);
    auto connection = sdbus::createSessionBusConnection();
    wlsunsetProxy client(
        *connection, "org.jjaro.wlsunset", "/org/jjaro/wlsunset",
        [](bool inhibit) { absl::PrintF("{\"text\": \"%1$d\", \"alt\": \"%2$s\", \"tooltip\": \"%2$s\", \"class\": \"%2$s\"}\n", inhibit ? 1 : 0, inhibit ? "inhibited" : "activated"); });
    absl::PrintF("{\"text\": \"%1$d\", \"alt\": \"%2$s\", \"tooltip\": \"%2$s\", \"class\": \"%2$s\"}\n", client.get_inhibit() ? 1 : 0, client.get_inhibit() ? "inhibited" : "activated");
    connection->enterEventLoop();
    return EXIT_SUCCESS;
  }
  absl::FPrintF(stderr,
                "Usage:\n"
                "  %1$s get_inhibit\n"
                "  %1$s set_inhibit <0|1>\n"
                "  %1$s toggle_inhibit\n"
                "  %1$s watch_inhibit\n",
                argc >= 1 && argv && argv[0] ? argv[0] : "wlsunsetctl");
  return EXIT_FAILURE;
}
