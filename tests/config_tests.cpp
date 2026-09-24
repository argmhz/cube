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

// The live frame stream is opt-in: nothing on the Pi should ever start
// listening or serializing frames just because the flag exists.
TEST_CASE("parseArgs leaves the frame stream off by default") {
  Config config = parseArgs({"--port", "1234"});
  CHECK(config.streamPort == "");
  CHECK(config.streamFps == 30);
}

TEST_CASE("parseArgs reads the frame stream port and rate") {
  Config config = parseArgs({"--stream-port", "8421", "--stream-fps", "60"});
  CHECK(config.streamPort == "8421");
  CHECK(config.streamFps == 60);
}

TEST_CASE("parseArgs keeps the default rate when given a useless one") {
  // 0 would be a division by zero picking the frame interval.
  Config config = parseArgs({"--stream-fps", "0"});
  CHECK(config.streamFps == 30);
}
