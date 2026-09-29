#pragma once

#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "Log.h"

/**
 * Reads the first non-loopback IPv4 address directly from the kernel via
 * getifaddrs() -- no shell, no subprocess. Unlike the `ip a | awk | ...`
 * pipeline this replaces, it doesn't depend on those binaries being present
 * with a specific output format, and is cheap enough to call in a tight
 * retry loop (see getIpAddress() below).
 */
inline std::string readIpAddressOnce(){
  struct ifaddrs *ifaddr;
  if (getifaddrs(&ifaddr) == -1) {
    return "";
  }

  std::string result;
  for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
    if (strcmp(ifa->ifa_name, "lo") == 0) continue;

    struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
    char host[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr->sin_addr, host, sizeof(host))) {
      result = host;
      break;
    }
  }

  freeifaddrs(ifaddr);
  return result;
}

/**
 * Get the IP address, retrying for a few seconds if it's not there yet.
 * On WiFi in particular, association + DHCP after boot can take a moment,
 * so the very first call can easily happen before an address is assigned --
 * retry instead of giving up immediately (bounded to ~10s so we still
 * return promptly when there's genuinely no network).
 * @return char *
 */
inline const char * getIpAddress(){
  static char w[128];

  for (int attempt = 0; attempt < 20; attempt++) {
    std::string ip = readIpAddressOnce();
    if (!ip.empty()) {
      if (attempt > 0) {
        Log::info("network address found after " + std::to_string(attempt) + " retries: " + ip);
      }
      strncpy(w, ip.c_str(), sizeof(w) - 1);
      w[sizeof(w) - 1] = '\0';
      return w;
    }
    if (attempt == 0) {
      Log::info("no network address yet, waiting...");
    }
    usleep(500000); // 500ms
  }

  Log::warn("gave up waiting for a network address");
  w[0] = '\0';
  return w;
}
