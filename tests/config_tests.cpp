#include "../lib/vendor/doctest.h"
#include "../lib/net/Config.h"

TEST_CASE("parseArgs with no arguments keeps the historical defaults") {
  Config config = parseArgs({});
  CHECK(config.host == "localhost");
  CHECK(config.port == "1234");
  CHECK(config.animationsDir == "./bin/animations");
}

TEST_CASE("parseArgs overrides only the flags that were given") {
  Config config = parseArgs({"--port", "9999"});
  CHECK(config.host == "localhost");
  CHECK(config.port == "9999");
  CHECK(config.animationsDir == "./bin/animations");
}

TEST_CASE("parseArgs overrides host and animations dir together") {
  Config config = parseArgs({"--host", "0.0.0.0", "--animations-dir", "/opt/cube/animations"});
  CHECK(config.host == "0.0.0.0");
  CHECK(config.port == "1234");
  CHECK(config.animationsDir == "/opt/cube/animations");
}

TEST_CASE("parseArgs ignores unknown flags without disturbing the rest") {
  Config config = parseArgs({"--bogus", "value", "--port", "5555"});
  CHECK(config.port == "5555");
}

TEST_CASE("parseArgs ignores a flag with no following value instead of crashing") {
  Config config = parseArgs({"--port"});
  CHECK(config.port == "1234");
}
