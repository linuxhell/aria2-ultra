#ifndef D_MULTICAST_TEST_SUPPORT_H
#define D_MULTICAST_TEST_SUPPORT_H

#ifndef __MINGW32__
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#endif

namespace aria2 {

// The LPD socket tests require a real, active multicast IPv4 interface.
// Minimal containers may expose only loopback, where joining the LPD group
// fails independently of the code under test.
inline bool hasMulticastTestInterface()
{
#ifdef __MINGW32__
  return true;
#else
  ifaddrs* addresses = nullptr;
  if (getifaddrs(&addresses) != 0) return false;
  bool available = false;
  for (auto* addr = addresses; addr; addr = addr->ifa_next) {
    if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_INET &&
        (addr->ifa_flags & IFF_UP) && (addr->ifa_flags & IFF_MULTICAST) &&
        !(addr->ifa_flags & IFF_LOOPBACK)) {
      available = true;
      break;
    }
  }
  freeifaddrs(addresses);
  return available;
#endif
}

} // namespace aria2

#endif // D_MULTICAST_TEST_SUPPORT_H
