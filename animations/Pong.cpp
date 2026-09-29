#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

using json = nlohmann::json;

// Up to four players, each defending one of the cube's four side walls
// with a paddle that slides up/down; top and bottom are always fixed
// walls. Miss the ball on your own wall and you lose one of three lives;
// lose them all and your wall just becomes a permanent wall for the rest
// of the round (its paddle stops being drawn). Same "always simulate all
// four, an unpiloted paddle just sits centred and quickly loses its
// lives" design as LightCycles.cpp -- see that file for why there's no
// join/leave message to invent for this protocol.
class Pong : public Animation {
  static constexpr int MAX_PLAYERS = 4;
  static constexpr int START_LIVES = 3;
  // Both the ball-hit test and the drawn patch use this: a paddle is a
  // roughly 3x3 square, its horizontal position fixed at the centre of
  // its wall, its vertical position (paddleY) the only thing a player
  // controls.
  static constexpr float PADDLE_HALF = 1.5f;
  static constexpr float WALL_CENTER = 3.5f;

  struct Rgb {
    int r, g, b;
  };

  // Same four identities as LightCycles.cpp, so a player keeps "their"
  // colour across games: 0=+X, 1=-X, 2=+Z, 3=-Z.
  static constexpr Rgb PLAYER_COLOR[MAX_PLAYERS] = {
      {0, 15, 15},  // cyan
      {15, 0, 15},  // magenta
      {15, 8, 0},   // amber
      {0, 15, 0},   // green
  };

  int lives[MAX_PLAYERS] = {};
  float paddleY[MAX_PLAYERS] = {};

  float bx = WALL_CENTER, by = WALL_CENTER, bz = WALL_CENTER;
  float vx = 0, vy = 0, vz = 0;

  int speed = 55000;

  static float randf(float lo, float hi) {
    return lo + (static_cast<float>(rand() % 10000) / 10000.0f) * (hi - lo);
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["player"].is_number() && data["dir"].is_number()) {
      int p = data["player"].get<int>() - 1;
      int d = data["dir"].get<int>();
      if (p >= 0 && p < MAX_PLAYERS && lives[p] > 0) {
        if (d == 1) {
          paddleY[p] = std::min(7.0f, paddleY[p] + 1.0f);
        } else if (d == 2) {
          paddleY[p] = std::max(0.0f, paddleY[p] - 1.0f);
        }
      }
    }
  }

  void launchBall() {
    bx = by = bz = WALL_CENTER;
    auto pick = [&]() {
      float m = randf(0.18f, 0.32f);
      return (rand() % 2 == 0) ? m : -m;
    };
    vx = pick();
    vy = pick();
    vz = pick();
  }

  void resetRound() {
    for (int p = 0; p < MAX_PLAYERS; p++) {
      lives[p] = START_LIVES;
      paddleY[p] = WALL_CENTER;
    }
    launchBall();
  }

  static bool onPaddle(float a, float aCenter, float b, float bCenter) {
    return std::fabs(a - aCenter) <= PADDLE_HALF && std::fabs(b - bCenter) <= PADDLE_HALF;
  }

  void announceRoundEnd(Cube *cube, int winner) {
    const Rgb &flash = PLAYER_COLOR[winner];
    for (int i = 0; i < 4; i++) {
      cube->clear();
      cube->update();
      usleep(150000);
      cube->all(flash.r, flash.g, flash.b);
      cube->update();
      usleep(150000);
    }
    cube->clear();
    cube->update();
    usleep(400000);
  }

  void draw(Cube *cube) override {
    resetRound();

    while (isRunning()) {
      bx += vx;
      by += vy;
      bz += vz;

      // X walls: player 0 defends +X, player 1 defends -X. In-plane axes
      // for this pair are z (fixed paddle centre) and y (paddleY).
      if (bx > 7.0f || bx < 0.0f) {
        const int p = bx > 7.0f ? 0 : 1;
        if (lives[p] > 0) {
          if (onPaddle(bz, WALL_CENTER, by, paddleY[p])) {
            vx = -vx;
            bx = std::clamp(bx, 0.0f, 7.0f);
          } else {
            lives[p]--;
            launchBall();
          }
        } else {
          vx = -vx;
          bx = std::clamp(bx, 0.0f, 7.0f);
        }
      }

      // Z walls: player 2 defends +Z, player 3 defends -Z. In-plane axes
      // are x (fixed paddle centre) and y (paddleY).
      if (bz > 7.0f || bz < 0.0f) {
        const int p = bz > 7.0f ? 2 : 3;
        if (lives[p] > 0) {
          if (onPaddle(bx, WALL_CENTER, by, paddleY[p])) {
            vz = -vz;
            bz = std::clamp(bz, 0.0f, 7.0f);
          } else {
            lives[p]--;
            launchBall();
          }
        } else {
          vz = -vz;
          bz = std::clamp(bz, 0.0f, 7.0f);
        }
      }

      // Y is always a wall -- nobody defends the top or bottom.
      if (by > 7.0f || by < 0.0f) {
        vy = -vy;
        by = std::clamp(by, 0.0f, 7.0f);
      }

      cube->clear();

      for (int p = 0; p < MAX_PLAYERS; p++) {
        if (lives[p] <= 0) {
          continue;
        }
        // Dim the paddle as lives run out -- cheap, useful feedback
        // without needing a separate life-counter display.
        const float wear = static_cast<float>(lives[p]) / START_LIVES;
        const Rgb &c = PLAYER_COLOR[p];
        const int r = static_cast<int>(std::lround(c.r * wear));
        const int g = static_cast<int>(std::lround(c.g * wear));
        const int b = static_cast<int>(std::lround(c.b * wear));

        const int yLo = static_cast<int>(std::lround(paddleY[p] - PADDLE_HALF));
        const int yHi = static_cast<int>(std::lround(paddleY[p] + PADDLE_HALF));
        const int cLo = static_cast<int>(std::lround(WALL_CENTER - PADDLE_HALF));
        const int cHi = static_cast<int>(std::lround(WALL_CENTER + PADDLE_HALF));

        for (int y = yLo; y <= yHi; y++) {
          for (int c2 = cLo; c2 <= cHi; c2++) {
            if (p == 0) {
              cube->set(7, y, c2, r, g, b);
            } else if (p == 1) {
              cube->set(0, y, c2, r, g, b);
            } else if (p == 2) {
              cube->set(c2, y, 7, r, g, b);
            } else {
              cube->set(c2, y, 0, r, g, b);
            }
          }
        }
      }

      cube->set(static_cast<int>(std::lround(std::clamp(bx, 0.0f, 7.0f))),
                static_cast<int>(std::lround(std::clamp(by, 0.0f, 7.0f))),
                static_cast<int>(std::lround(std::clamp(bz, 0.0f, 7.0f))), MAX_COLOR, MAX_COLOR,
                MAX_COLOR);

      cube->update();

      int aliveCount = 0;
      int lastAlive = -1;
      for (int p = 0; p < MAX_PLAYERS; p++) {
        if (lives[p] > 0) {
          aliveCount++;
          lastAlive = p;
        }
      }
      if (aliveCount <= 1) {
        announceRoundEnd(cube, lastAlive);
        resetRound();
      }

      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new Pong;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
