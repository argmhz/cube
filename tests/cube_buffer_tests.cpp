#include "../lib/doctest.h"
#include "../lib/CubeBuffer.h"

// These tests exercise the pure voxel-buffer/geometry logic in CubeBuffer.
// No hardware, no root, no Raspberry Pi involved -- safe to run anywhere.

TEST_CASE("inBounce accepts only coordinates inside the 8x8x8 cube") {
  CubeBuffer cube;
  CHECK(cube.inBounce(0,0,0));
  CHECK(cube.inBounce(7,7,7));
  CHECK_FALSE(cube.inBounce(-1,0,0));
  CHECK_FALSE(cube.inBounce(8,0,0));
  CHECK_FALSE(cube.inBounce(0,0,8));
}

TEST_CASE("set(x,y,z,r,g,b) then get(x,y,z) returns the same color") {
  CubeBuffer cube;
  cube.set(3,4,5, 7,8,9);
  CubeBuffer::Color c = cube.get(3,4,5);
  CHECK(c.red == 7);
  CHECK(c.green == 8);
  CHECK(c.blue == 9);
}

TEST_CASE("set(x,y,z,r,g,b) outside the cube is ignored, not out of bounds") {
  CubeBuffer cube;
  cube.set(8,0,0, 15,15,15); // out of range, must be a no-op
  CHECK(cube.isOff(0,0,0));
}

TEST_CASE("set(x,y,z,Color) must place the voxel at the given z, not overwrite it with x") {
  CubeBuffer cube;
  CubeBuffer::Color color;
  color.set(1,2,3);

  cube.set(2,2,6, color); // x=2, z=6 -- deliberately different so a z<->x mixup is visible

  CubeBuffer::Color atIntendedPosition = cube.get(2,2,6);
  CHECK(atIntendedPosition.red == 1);
  CHECK(atIntendedPosition.green == 2);
  CHECK(atIntendedPosition.blue == 3);
}

TEST_CASE("clear() turns every voxel off") {
  CubeBuffer cube;
  cube.all(15,15,15);
  cube.clear();
  CHECK(cube.isOff(0,0,0));
  CHECK(cube.isOff(7,7,7));
}

TEST_CASE("all(r,g,b) lights every voxel in the cube") {
  CubeBuffer cube;
  cube.all(4,5,6);
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      for (int z = 0; z < 8; z++) {
        CubeBuffer::Color c = cube.get(x,y,z);
        CHECK(c.red == 4);
        CHECK(c.green == 5);
        CHECK(c.blue == 6);
      }
    }
  }
}

TEST_CASE("line() lights both of its endpoints") {
  CubeBuffer cube;
  cube.line(0,0,0, 7,7,7, 15,15,15);
  CHECK(cube.get(0,0,0).red == 15);
  CHECK(cube.get(7,7,7).red == 15);
}

TEST_CASE("plane() lights an entire axis-aligned slice") {
  CubeBuffer cube;
  cube.plane(AXIS_Z, 3, 9,9,9);
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      CHECK(cube.get(x,y,3).red == 9);
    }
  }
  // A neighbouring slice must stay untouched
  CHECK(cube.isOff(0,0,4));
}

TEST_CASE("box() fills every voxel in the given range, including reversed bounds") {
  CubeBuffer cube;
  cube.box(3,3,3, 1,1,1, 10,10,10); // start > end on purpose, box() must handle it
  for (int x = 1; x <= 3; x++) {
    for (int y = 1; y <= 3; y++) {
      for (int z = 1; z <= 3; z++) {
        CHECK(cube.get(x,y,z).red == 10);
      }
    }
  }
  CHECK(cube.isOff(0,0,0));
}

TEST_CASE("boxOutline() lights edges but not face centers") {
  CubeBuffer cube;
  // Deliberately asymmetric box (endx != endy) to expose an endx/endy mixup.
  cube.boxOutline(0,0,0, 5,3,5, 15,15,15);

  CHECK(cube.get(0,0,0).red == 15); // corner: on an edge

  // x=3 is interior in x (0..5) and not on any x/z boundary, only on the
  // y=starty(0) face -- a face center, not an edge, so it must stay dark.
  CubeBuffer::Color faceCenter = cube.get(3,0,2);
  CHECK(faceCenter.red == 0);
}

TEST_CASE("shift() moves voxels one step along an axis and clears the vacated end") {
  CubeBuffer cube;
  cube.set(3,3,3, 12,13,14);
  cube.shift(AXIS_Z, -1);
  CHECK(cube.get(3,3,2).red == 12);
  CHECK(cube.isOff(3,3,3));
}

TEST_CASE("rotate(axis, 0) is a no-op") {
  CubeBuffer cube;
  cube.set(5,2,6, 9,10,11);
  cube.rotate(AXIS_Z, 0);
  CubeBuffer::Color c = cube.get(5,2,6);
  CHECK(c.red == 9);
  CHECK(c.green == 10);
  CHECK(c.blue == 11);
}

TEST_CASE("rotate() rotates around the cube's center, not the corner (0,0,0)") {
  CubeBuffer cube;
  cube.set(3,3,3, 15,15,15); // the voxel nearest the cube's own center
  cube.rotate(AXIS_Z, 90);
  // A point already at the center must stay right next to it, not swing
  // away toward the (0,0,0) corner as it would if rotation used raw
  // 0..7 coordinates instead of coordinates centered on the cube.
  CHECK(cube.get(4,3,3).red == 15);
}

TEST_CASE("rotate(AXIS_Z, 90) rotates a point the expected quarter turn") {
  CubeBuffer cube;
  cube.set(7,3,3, 15,15,15);
  cube.rotate(AXIS_Z, 90);
  CHECK(cube.get(4,7,3).red == 15);
}

TEST_CASE("rotate(AXIS_X, 90) rotates a point the expected quarter turn") {
  CubeBuffer cube;
  cube.set(3,7,3, 15,15,15);
  cube.rotate(AXIS_X, 90);
  CHECK(cube.get(3,4,7).red == 15);
}

TEST_CASE("rotate(AXIS_Y, 90) rotates a point the expected quarter turn") {
  // This is the axis whose hand-derived formula used to mix up y and z.
  CubeBuffer cube;
  cube.set(7,3,3, 15,15,15);
  cube.rotate(AXIS_Y, 90);
  CHECK(cube.get(3,3,0).red == 15);
}

TEST_CASE("rotateZ() delegates to rotate(AXIS_Z, ...) and matches it") {
  CubeBuffer cube;
  cube.set(7,3,3, 15,15,15);
  cube.rotateZ(90);
  CHECK(cube.get(4,7,3).red == 15);
}

TEST_CASE("rotate() with an extreme angle drops out-of-range voxels instead of corrupting state") {
  CubeBuffer cube;
  cube.all(1,1,1);
  // Before the bounds fix, angles like this wrote far outside the local
  // 8x8x8 staging array used internally by rotate() -- undefined behaviour.
  // Every voxel started at (1,1,1); after rotating, each one is either
  // still (1,1,1) (mapped to a valid position) or was dropped and is back
  // at the cleared (0,0,0) -- never anything else, and never a crash.
  cube.rotate(AXIS_X, 999999);
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      for (int z = 0; z < 8; z++) {
        CubeBuffer::Color c = cube.get(x,y,z);
        CHECK((c.red == 0 || c.red == 1));
        CHECK(c.red == c.green);
        CHECK(c.red == c.blue);
      }
    }
  }
}
