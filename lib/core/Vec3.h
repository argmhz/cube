#pragma once

#include <cmath>
#include "CubeBuffer.h" // AXIS_X/AXIS_Y/AXIS_Z

// Small, hardware-free 3D vector + rotation helper. Exists so rotation math
// lives in exactly one, tested place instead of being hand-derived per axis
// (which is how the AXIS_Y formula in CubeBuffer::rotate() ended up wrong).
struct Vec3 {
  double x = 0;
  double y = 0;
  double z = 0;
};

// Standard right-handed rotation matrices around the X/Y/Z axis, by `radians`.
inline Vec3 rotateAroundAxis(Vec3 v, int axis, double radians) {
  double cosT = cos(radians);
  double sinT = sin(radians);

  switch (axis) {
    case AXIS_X:
      return Vec3{ v.x, v.y * cosT - v.z * sinT, v.y * sinT + v.z * cosT };
    case AXIS_Y:
      return Vec3{ v.x * cosT + v.z * sinT, v.y, -v.x * sinT + v.z * cosT };
    case AXIS_Z:
      return Vec3{ v.x * cosT - v.y * sinT, v.x * sinT + v.y * cosT, v.z };
    default:
      return v;
  }
}
