#include "../lib/vendor/doctest.h"
#include "../lib/remotehelpers.h"

// readIpAddressOnce() reads real OS network state (via getifaddrs()), so
// its actual content isn't something a test can assert on -- this is a
// smoke test confirming it doesn't crash and returns cleanly, hardware-free.
// getIpAddress()'s ~10s retry loop is deliberately not exercised here, to
// keep the suite fast.

TEST_CASE("readIpAddressOnce does not crash and returns a string") {
  std::string ip = readIpAddressOnce();
  CHECK(ip.size() < 128);
}
