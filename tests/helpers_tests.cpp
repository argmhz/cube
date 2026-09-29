#include "../lib/vendor/doctest.h"
#include "../lib/helpers.h"
#include <cstdlib>

// Pure numeric/color helpers -- no hardware involved.

TEST_CASE("random(max) always returns a value in [0, max)") {
  srand(1);
  for (int i = 0; i < 200; i++) {
    int value = random(16);
    CHECK(value >= 0);
    CHECK(value < 16);
  }
}

TEST_CASE("random(min, max) must stay within [min, max), not just start at min") {
  srand(1);
  // With max-min == 1, every draw must land on exactly `min`.
  bool allExactlyMin = true;
  int maxSeen = 3;
  for (int i = 0; i < 50; i++) {
    int value = random(3, 4);
    if (value != 3) allExactlyMin = false;
    if (value > maxSeen) maxSeen = value;
  }
  CHECK(allExactlyMin);
  CHECK(maxSeen == 3);
}

TEST_CASE("random(min, max) never returns max or above") {
  srand(1);
  int minSeen = 1000;
  int maxSeen = 999;
  for (int i = 0; i < 500; i++) {
    int value = random(1000, 2000000);
    if (value < minSeen) minSeen = value;
    if (value > maxSeen) maxSeen = value;
  }
  CHECK(minSeen >= 1000);
  CHECK(maxSeen < 2000000);
}

TEST_CASE("dmap linearly remaps a value between ranges") {
  CHECK(dmap(5, 0, 10, 0, 100) == doctest::Approx(50));
  CHECK(dmap(0, 0, 10, 0, 100) == doctest::Approx(0));
  CHECK(dmap(10, 0, 10, 0, 100) == doctest::Approx(100));
}

TEST_CASE("map linearly remaps an integer value between ranges") {
  CHECK(map(128, 0, 255, 0, 15) == 7);
  CHECK(map(0, 0, 255, 0, 15) == 0);
  CHECK(map(255, 0, 255, 0, 15) == 15);
}

TEST_CASE("colorConverter converts a hex string into a 0-15 scaled color") {
  CubeBuffer::Color c = colorConverter("ff0080");
  CHECK(c.red == 15);
  CHECK(c.green == 0);
  CHECK(c.blue == map(0x80, 0, 255, 0, 15));
}

TEST_CASE("fadeColor produces the requested number of steps, starting at the first color") {
  std::vector<CubeBuffer::Color> steps = fadeColor(0,0,0, 15,15,15, 4);
  CHECK(steps.size() == 4);
  CHECK(steps[0].red == 0);
  CHECK(steps[0].green == 0);
  CHECK(steps[0].blue == 0);
}
