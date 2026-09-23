#include "../lib/vendor/doctest.h"
#include "../lib/core/Cube.h"
#include "../lib/animation/Font.h"

#include <string>

// fonts[] has 128 entries, so any byte the renderer passes to it must be
// below 128. Text typed into the web UI arrives as UTF-8, where every
// Danish letter is two bytes that are both above it.

TEST_CASE("renderable never emits a byte the font table cannot index") {
  const std::string inputs[] = {
      "Så",          // å
      "ÆØÅ",         // uppercase Danish
      "æøå",         // lowercase Danish
      "Topper 3D",   // plain ASCII
      "hej 🎉 der",  // a four-byte sequence the font has no glyph for
      "",
  };

  for (const std::string &input : inputs) {
    for (unsigned char byte : Font::renderable(input)) {
      CHECK(byte < 128);
    }
  }
}

TEST_CASE("renderable transliterates the Danish letters rather than dropping them") {
  CHECK(Font::renderable("Så") == "Saa");
  CHECK(Font::renderable("æøå") == "aeoeaa");
  CHECK(Font::renderable("ÆØÅ") == "AEOEAA");
}

TEST_CASE("renderable leaves ASCII untouched") {
  CHECK(Font::renderable("Topper 3D ") == "Topper 3D ");
  CHECK(Font::renderable("") == "");
}

TEST_CASE("renderable drops unknown multi-byte characters whole") {
  // The emoji must not leave any of its continuation bytes behind.
  CHECK(Font::renderable("hej 🎉 der") == "hej  der");
}
