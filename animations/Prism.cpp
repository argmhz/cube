#include "../lib/core/Cube.h"
#include "../lib/core/Vec3.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

// A wireframe cube tumbling in place, its edges cycling through a smooth
// rainbow (every brightness level is real thanks to bit-angle modulation --
// this leans on that instead of only ever using full brightness), with a
// softly pulsing core at the center.
//
// The rotation reuses Vec3.h's rotateAroundAxis() -- the same matrix math
// CubeBuffer::rotate() uses -- applied straight to the 8 original corner
// points every frame, rather than repeatedly calling CubeBuffer::rotate()
// on the cube's own voxel buffer. CubeBuffer::rotate() is built for a single
// manual "rotate what's on screen right now" command; calling it every
// frame forever would re-round already-rounded voxel positions each time,
// and the wireframe would visibly degrade the longer this animation runs.
// Rotating the clean, original geometry fresh each frame avoids that
// entirely -- the cube stays crisp no matter how long it spins.
class Prism : public Animation {

  int speed = 30000;

  void onDataUpdate(json data){
    if(data["speed"].is_number()){
      speed = data["speed"].get<int>();
    }
  }

  void draw(Cube *cube) {

    const double CENTER = 3.5;
    // Half-extent kept small enough that even the cube's far diagonal
    // corner (distance SIZE*sqrt(3) from center) never rotates outside the
    // 0..7 grid -- otherwise corners would silently vanish for part of
    // every rotation as they swing past the edge of the cube.
    const double SIZE = 2.0;

    Vec3 baseCorners[8] = {
      {-SIZE,-SIZE,-SIZE}, { SIZE,-SIZE,-SIZE}, { SIZE, SIZE,-SIZE}, {-SIZE, SIZE,-SIZE},
      {-SIZE,-SIZE, SIZE}, { SIZE,-SIZE, SIZE}, { SIZE, SIZE, SIZE}, {-SIZE, SIZE, SIZE},
    };

    int edges[12][2] = {
      {0,1},{1,2},{2,3},{3,0},
      {4,5},{5,6},{6,7},{7,4},
      {0,4},{1,5},{2,6},{3,7},
    };

    double angleX = 0, angleY = 0, angleZ = 0;
    int colorIndex = 0;
    int pulsePhase = 0;

    while(isRunning()){

      cube->clear();

      Vec3 rotated[8];
      for (int i = 0; i < 8; i++) {
        Vec3 p = baseCorners[i];
        p = rotateAroundAxis(p, AXIS_X, angleX * PI / 180.0);
        p = rotateAroundAxis(p, AXIS_Y, angleY * PI / 180.0);
        p = rotateAroundAxis(p, AXIS_Z, angleZ * PI / 180.0);
        rotated[i] = p;
      }

      for (int e = 0; e < 12; e++) {
        Vec3 a = rotated[edges[e][0]];
        Vec3 b = rotated[edges[e][1]];

        Cube::Color c = makeColorGradient(colorIndex + e * 10);

        cube->line(
          (int)round(a.x + CENTER), (int)round(a.y + CENTER), (int)round(a.z + CENTER),
          (int)round(b.x + CENTER), (int)round(b.y + CENTER), (int)round(b.z + CENTER),
          c.red, c.green, c.blue
        );
      }

      // Pulsing core: a breathing sphere of white light at the very center.
      double pulse = (sin(pulsePhase * 0.05) + 1.0) / 2.0; // 0..1
      int coreBrightness = (int)round(pulse * 15);
      cube->sphere(4, 4, 4, 1, coreBrightness, coreBrightness, coreBrightness);

      cube->update();

      angleX = fmod(angleX + 1.3, 360.0);
      angleY = fmod(angleY + 0.8, 360.0);
      angleZ = fmod(angleZ + 0.5, 360.0);
      colorIndex += 2;
      pulsePhase++;

      usleep(speed);
    }
  }

};
extern "C" Animation * create() {
    return new Prism;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
