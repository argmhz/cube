#include "../lib/vendor/doctest.h"
#include "../animations/LightCycles.cpp"

// isReversal() is the one piece of LightCycles' game logic that doesn't
// need a running Cube to exercise -- it's what stands between "the round
// ends the instant someone double-taps their own direction" and a normal
// game. 0..5 encodes +X,-X,+Y,-Y,+Z,-Z (see AXIS_D* in LightCycles.cpp).

TEST_CASE("isReversal rejects turning straight back along each axis") {
  CHECK(LightCycles::isReversal(0, 1));  // +X -> -X
  CHECK(LightCycles::isReversal(1, 0));  // -X -> +X
  CHECK(LightCycles::isReversal(2, 3));  // +Y -> -Y
  CHECK(LightCycles::isReversal(3, 2));  // -Y -> +Y
  CHECK(LightCycles::isReversal(4, 5));  // +Z -> -Z
  CHECK(LightCycles::isReversal(5, 4));  // -Z -> +Z
}

TEST_CASE("isReversal accepts every real turn") {
  for (int current = 0; current < 6; current++) {
    for (int candidate = 0; candidate < 6; candidate++) {
      bool sameAxis = current / 2 == candidate / 2;
      if (sameAxis) {
        continue;  // covered by the rejection test above
      }
      CHECK_FALSE(LightCycles::isReversal(current, candidate));
    }
  }
}

TEST_CASE("isReversal accepts continuing straight (not a turn, but not a reversal either)") {
  for (int d = 0; d < 6; d++) {
    CHECK_FALSE(LightCycles::isReversal(d, d));
  }
}
