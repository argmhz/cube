#include "../lib/doctest.h"
#include "../lib/AniManager.cpp"

// AniManager's methods are all defined inline inside the class body (same
// as every apps/*.cpp already does), so this file can #include the .cpp
// directly without any linker conflicts.
//
// `cube` is only ever stored as an opaque pointer here, never dereferenced,
// so passing nullptr is safe for these tests.

TEST_CASE("a fresh AniManager is not ready, and destructing it is safe") {
  AniManager manager(nullptr);
  CHECK_FALSE(manager.isReady());
}

TEST_CASE("stopAnimation() before anything has loaded does not crash") {
  AniManager manager(nullptr);
  manager.stopAnimation();
  CHECK_FALSE(manager.isReady());
}

TEST_CASE("loadAnimation() with a bogus path leaves the manager not ready, not crashed") {
  AniManager manager(nullptr);
  manager.loadAnimation("./this-file-does-not-exist.so");
  CHECK_FALSE(manager.isReady());
  manager.stopAnimation(); // must still be safe after a failed load
}
