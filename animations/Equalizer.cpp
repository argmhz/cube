#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <random>
#include <unistd.h>

using json = nlohmann::json;

// A spectrum analyser dancing to music. There is no microphone on the cube,
// so the music is made up: a four-on-the-floor beat at `tempo` with a kick
// in the bass, a snare in the mids on beats two and four, hi-hats up top and
// a wandering melody on top of that, arranged in 16-bar phrases that end in
// a breakdown before the beat drops back in.
//
// Eight frequency bands, bass first, shown three ways:
// - Waterfall: bands side by side along x, the current spectrum at the front
//   and older ones drifting back through the cube, so the music leaves a
//   landscape behind it.
// - Classic: eight solid bars, each with a peak marker that hangs a moment
//   before it drops, like a hi-fi.
// - Rings: square rings on the floor, bass in the middle and treble at the
//   edge, so every kick pumps the centre.
class Equalizer : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  static constexpr int BANDS = 8;
  static constexpr int MODE_WATERFALL = 0;
  static constexpr int MODE_CLASSIC = 1;
  static constexpr int MODE_RINGS = 2;
  static constexpr int PALETTE_METER = 0;
  static constexpr int PALETTE_RAINBOW = 1;

  // How fast bars fall back, in full heights per second. They jump up to a
  // new level at once, like a real analyser's ballistics.
  static constexpr float FALL_RATE = 1.6f;
  static constexpr float PEAK_HOLD = 0.35f;
  static constexpr float PEAK_FALL = 0.9f;
  // Seconds between rows of the waterfall moving one step back.
  static constexpr float WATERFALL_STEP = 0.09f;

  // Set from the connection thread while draw() runs.
  std::atomic<int> mode{MODE_WATERFALL};
  std::atomic<int> palette{PALETTE_METER};
  std::atomic<float> tempo{120.0f};
  std::atomic<float> gain{1.0f};
  std::atomic<float> brightness{1.0f};

  std::mt19937 random{std::random_device{}()};

  float level[BANDS] = {};
  float peak[BANDS] = {};
  float peakAge[BANDS] = {};
  // Row 0 is the newest, at the front.
  float history[8][BANDS] = {};

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
  }

  static float wrap(float value) {
    return value - std::floor(value);
  }

  static Rgb rainbow(float hue) {
    hue = wrap(hue) * 6.0f;
    const int sector = static_cast<int>(std::floor(hue));
    const float fraction = hue - sector;

    switch (sector % 6) {
      case 0: return {1.0f, fraction, 0.0f};
      case 1: return {1.0f - fraction, 1.0f, 0.0f};
      case 2: return {0.0f, 1.0f, fraction};
      case 3: return {0.0f, 1.0f - fraction, 1.0f};
      case 4: return {fraction, 0.0f, 1.0f};
      default: return {1.0f, 0.0f, 1.0f - fraction};
    }
  }

  // The classic level meter: green, then yellow and orange, red at the top.
  static Rgb meter(int row) {
    if (row <= 3) return {0.0f, 1.0f, 0.0f};
    if (row <= 5) return {1.0f, 0.8f, 0.0f};
    if (row == 6) return {1.0f, 0.35f, 0.0f};
    return {1.0f, 0.0f, 0.0f};
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["mode"].is_number()) {
      mode = std::clamp(data["mode"].get<int>(), 0, 2);
    }
    if (data["palette"].is_number()) {
      palette = std::clamp(data["palette"].get<int>(), 0, 1);
    }
    if (data["tempo"].is_number()) {
      tempo = std::clamp(data["tempo"].get<float>(), 60.0f, 180.0f);
    }
    if (data["gain"].is_number()) {
      gain = std::clamp(data["gain"].get<float>(), 0.3f, 1.5f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  // What the made-up song sounds like in each band right now, 0 to about 1.
  void listen(double beat, float time, float (&target)[BANDS]) {
    const float phase = static_cast<float>(beat - std::floor(beat));
    const long whole = static_cast<long>(std::floor(beat));
    const int beatInBar = static_cast<int>(whole % 4);
    const int barInPhrase = static_cast<int>((whole / 4) % 16);

    // The last bar of every phrase drops the drums and leaves the melody,
    // and the bar before it builds with doubled hi-hats.
    const bool breakdown = barInPhrase == 15;
    const bool buildUp = barInPhrase == 14;

    const float kick = breakdown ? 0.0f : std::exp(-phase * 6.0f);
    const float snare = !breakdown && (beatInBar == 1 || beatInBar == 3) ? std::exp(-phase * 8.0f) : 0.0f;
    const float hatPhase = static_cast<float>(wrap(beat * (buildUp ? 4.0 : 2.0)));
    const float hat = breakdown ? 0.0f : 0.75f * std::exp(-hatPhase * 14.0f);
    const float bass = breakdown ? 0.15f : 0.3f + 0.1f * std::sin(time * 1.3f);

    target[0] = bass + 0.7f * kick;
    target[1] = bass * 0.9f + 0.6f * kick;
    target[2] = 0.15f + 0.35f * kick + 0.25f * snare;
    target[3] = 0.1f + 0.75f * snare;
    target[4] = 0.1f + 0.65f * snare;
    target[5] = 0.05f + 0.35f * snare + 0.35f * hat;
    target[6] = 0.05f + 0.85f * hat;
    target[7] = 0.9f * hat;

    // The melody: a hump of energy that wanders up and down the bands.
    const float centre = 3.5f + 2.3f * std::sin(time * 0.9f) + 0.8f * std::sin(time * 2.3f);
    const float loudness = (breakdown ? 0.75f : 0.45f) + 0.2f * std::sin(time * 0.37f);
    for (int b = 0; b < BANDS; b++) {
      const float d = (b - centre) / 1.3f;
      target[b] += loudness * std::exp(-d * d) + uniform(0.0f, 0.1f);
      target[b] *= gain;
    }
  }

  void column(Cube *cube, int x, int z, float height, Rgb band, int pal, float dim) {
    const float filled = height * 8.0f;
    for (int y = 0; y < 8; y++) {
      // The top voxel is lit only as far as the bar reaches into it, so the
      // bar rises and falls smoothly instead of in whole steps.
      const float amount = std::clamp(filled - y, 0.0f, 1.0f);
      if (amount <= 0.02f) {
        break;
      }
      const Rgb c = pal == PALETTE_METER ? meter(y) : band;
      const float k = amount * dim;
      cube->set(x, y, z, toLevel(c.r * k), toLevel(c.g * k), toLevel(c.b * k));
    }
  }

  void marker(Cube *cube, int x, int z, float height) {
    const int y = std::clamp(static_cast<int>(std::lround(height * 8.0f)), 0, 7);
    cube->set(x, y, z, toLevel(0.8f), toLevel(0.8f), toLevel(0.9f));
  }

  void draw(Cube *cube) override {
    using clock = std::chrono::steady_clock;
    // Real time, not a frame count, so the beat really is at `tempo`.
    auto last = clock::now();
    double beat = 0.0;
    float time = 0.0f;
    float sinceStep = 0.0f;

    while (isRunning()) {
      const auto now = clock::now();
      const float dt = std::min(0.1f, std::chrono::duration<float>(now - last).count());
      last = now;
      beat += dt * tempo / 60.0;
      time += dt;
      // Kept within a few hours' worth so float precision holds; 64 beats
      // is four whole phrases, so the song carries on where it was.
      if (beat > 64.0 * 1000.0) {
        beat -= 64.0 * 1000.0;
      }
      if (time > 10000.0f) {
        time -= 10000.0f;
      }

      float target[BANDS];
      listen(beat, time, target);
      for (int b = 0; b < BANDS; b++) {
        target[b] = std::clamp(target[b], 0.0f, 1.0f);
        level[b] = target[b] > level[b] ? target[b] : std::max(target[b], level[b] - FALL_RATE * dt);

        if (level[b] >= peak[b]) {
          peak[b] = level[b];
          peakAge[b] = 0.0f;
        } else {
          peakAge[b] += dt;
          if (peakAge[b] > PEAK_HOLD) {
            peak[b] = std::max(level[b], peak[b] - PEAK_FALL * dt);
          }
        }
      }

      sinceStep += dt;
      if (sinceStep >= WATERFALL_STEP) {
        sinceStep = 0.0f;
        for (int row = 7; row > 0; row--) {
          std::copy(history[row - 1], history[row - 1] + BANDS, history[row]);
        }
      }
      std::copy(level, level + BANDS, history[0]);

      const int currentMode = mode;
      const int pal = palette;
      cube->clear();

      switch (currentMode) {
        case MODE_WATERFALL:
          // z = 7 faces the viewer; older rows sit further back and dimmer.
          for (int row = 0; row < 8; row++) {
            for (int b = 0; b < BANDS; b++) {
              column(cube, b, 7 - row, history[row][b], rainbow(b / 9.0f), pal, 1.0f - row * 0.09f);
            }
          }
          break;

        case MODE_CLASSIC:
          for (int b = 0; b < BANDS; b++) {
            for (int z = 0; z < 8; z++) {
              column(cube, b, z, level[b], rainbow(b / 9.0f), pal, 1.0f);
            }
            if (peak[b] > level[b] + 0.1f) {
              for (int z = 0; z < 8; z++) {
                marker(cube, b, z, peak[b]);
              }
            }
          }
          break;

        default:
          // Four square rings, each showing the louder of a pair of bands.
          for (int x = 0; x < 8; x++) {
            for (int z = 0; z < 8; z++) {
              const int ring = std::max(std::abs(2 * x - 7), std::abs(2 * z - 7)) / 2;
              const int b = ring * 2;
              const float height = std::max(level[b], level[b + 1]);
              column(cube, x, z, height, rainbow(ring / 4.5f), pal, 1.0f);
              const float top = std::max(peak[b], peak[b + 1]);
              if (top > height + 0.1f) {
                marker(cube, x, z, top);
              }
            }
          }
          break;
      }

      cube->update();
      usleep(25000);
    }
  }
};
extern "C" Animation *create() {
  return new Equalizer;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
