#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <unistd.h>
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

using json = nlohmann::json;

// A microphone spectrum analyser laid out in rings around the cube's
// centre: bass in the middle, treble at the outer wall, each column rising
// from the floor (y = 0) by how loud its band currently is.
//
// The levels come from the browser -- cube-client captures the mic with an
// AnalyserNode, collapses the FFT into BANDS averages and streams them as
// {"action":"set","bands":[0-255 x4]} about 30 times a second. Nothing is
// analysed on the Pi; this animation only draws what it is told.
//
// The 8x8 floor gives exactly four Chebyshev rings around the 2x2 centre,
// which is where the band count comes from:
//
//   3 3 3 3 3 3 3 3     ring 0 =  4 columns -> bass
//   3 2 2 2 2 2 2 3     ring 1 = 12 columns -> low-mid
//   3 2 1 1 1 1 2 3     ring 2 = 20 columns -> high-mid
//   3 2 1 0 0 1 2 3     ring 3 = 28 columns -> treble
//   3 2 1 0 0 1 2 3
//   3 2 1 1 1 1 2 3
//   3 2 2 2 2 2 2 3
//   3 3 3 3 3 3 3 3
class Spectrum : public Animation {
  static constexpr int BANDS = 4;
  static constexpr int LEVEL_MAX = 255;

  // Written by onDataUpdate() on the connection thread, read by draw() on
  // the main thread -- every access goes through `levelsMutex`. Existing
  // animations get away with unsynchronised ints because a slider moves a
  // few times a second; at 30 updates a second on an array it is a real
  // data race.
  std::mutex levelsMutex;
  int levels[BANDS] = {};

  // Drawing state, touched only by draw().
  float heights[BANDS] = {};
  float peaks[BANDS] = {};

  int speed = 25000;
  // Percent applied to every incoming level before it is drawn.
  int gain = 100;
  // How fast a column falls once the sound stops, and how fast the peak
  // marker above it sinks, both in voxels per frame x100.
  int decay = 35;
  int peak_decay = 8;
  // Levels at or below this are treated as silence, so room noise does not
  // leave the bottom row permanently lit.
  int gate = 8;
  int show_peak = 1;

  // Ring index 0-3 for a floor position, from the Chebyshev distance to the
  // centre. Doubling puts the centre of an even grid on a whole number:
  // x = 3 and x = 4 both give 1, x = 0 and x = 7 both give 7.
  static int ringOf(int x, int z) {
    int dx = std::abs(2 * x - 7);
    int dz = std::abs(2 * z - 7);
    return (std::max(dx, dz) - 1) / 2;
  }

  // Bass reads warm, treble reads cold, so the eye can tell the rings
  // apart even when they are all lit at once.
  static void colorFor(int ring, int &r, int &g, int &b) {
    switch (ring) {
      case 0:  r = 15; g =  0; b =  0; break;  // bass      -- red
      case 1:  r = 15; g =  6; b =  0; break;  // low-mid   -- amber
      case 2:  r =  0; g = 15; b =  2; break;  // high-mid  -- green
      default: r =  0; g =  5; b = 15; break;  // treble    -- blue
    }
  }

  void onDataUpdate(json data) override {
    if (data["bands"].is_array()) {
      std::lock_guard<std::mutex> lock(levelsMutex);
      // Anything past BANDS is ignored and anything short leaves the
      // remaining bands alone, so a client sending the wrong length
      // degrades instead of reading out of bounds.
      int count = std::min((int)data["bands"].size(), BANDS);
      for (int i = 0; i < count; i++) {
        if (data["bands"][i].is_number()) {
          int value = data["bands"][i].get<int>();
          levels[i] = std::clamp(value, 0, LEVEL_MAX);
        }
      }
    }

    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }

    if (data["gain"].is_number()) {
      int value = data["gain"].get<int>();
      if (value > 0) {
        gain = value;
      }
    }

    if (data["decay"].is_number()) {
      int value = data["decay"].get<int>();
      if (value > 0) {
        decay = value;
      }
    }

    if (data["peak_decay"].is_number()) {
      int value = data["peak_decay"].get<int>();
      if (value > 0) {
        peak_decay = value;
      }
    }

    if (data["gate"].is_number()) {
      int value = data["gate"].get<int>();
      if (value >= 0) {
        gate = value;
      }
    }

    if (data["show_peak"].is_number()) {
      show_peak = data["show_peak"].get<int>() ? 1 : 0;
    }
  }

  void draw(Cube *c) override {
    while (isRunning()) {
      int current[BANDS];
      {
        std::lock_guard<std::mutex> lock(levelsMutex);
        for (int i = 0; i < BANDS; i++) {
          current[i] = levels[i];
        }
      }

      for (int i = 0; i < BANDS; i++) {
        int level = current[i] <= gate ? 0 : current[i];
        // 0-255 in, 0-8 voxels of column out.
        float target = (level * gain / 100.0f) * 8.0f / LEVEL_MAX;
        target = std::min(target, 8.0f);

        // Jump straight up to a new peak so a transient is not smeared,
        // then sink back gradually -- the same asymmetry a VU meter has.
        heights[i] = target >= heights[i] ? target : std::max(target, heights[i] - decay / 100.0f);
        peaks[i] = heights[i] >= peaks[i] ? heights[i] : std::max(heights[i], peaks[i] - peak_decay / 100.0f);
      }

      c->clear();

      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          int ring = ringOf(x, z);
          int r, g, b;
          colorFor(ring, r, g, b);

          int height = (int)heights[ring];
          for (int y = 0; y < height; y++) {
            c->set(x, y, z, r, g, b);
          }

          // The peak marker rides one voxel above the column in white, and
          // is skipped when it would sit inside the column it marks.
          int peak = (int)peaks[ring];
          if (show_peak && peak > height && peak < 8) {
            c->set(x, peak, z, 15, 15, 15);
          }
        }
      }

      c->update();
      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new Spectrum;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
