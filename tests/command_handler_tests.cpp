#include "../lib/vendor/doctest.h"
#include "../lib/net/CommandHandler.h"
#include "../lib/net/AnimationCommandHandler.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

// --- handleCommand() dispatch, tested against a fake handler -- no socket,
// no AniManager, no hardware involved. -------------------------------------

class FakeCommandHandler : public CommandHandler {
public:
  std::string lastSelected;
  int selectCalls = 0;
  json lastSetData;
  int setCalls = 0;
  std::vector<std::string> animationsToReturn;
  int listCalls = 0;

  void selectAnimation(const std::string &name) override {
    lastSelected = name;
    selectCalls++;
  }
  void setParams(const json &data) override {
    lastSetData = data;
    setCalls++;
  }
  std::vector<std::string> listAnimations() override {
    listCalls++;
    return animationsToReturn;
  }
};

TEST_CASE("handleCommand dispatches select") {
  FakeCommandHandler handler;
  auto reply = handleCommand(R"({"action":"select","animation":"ColorWheel"})", handler);
  CHECK(handler.selectCalls == 1);
  CHECK(handler.lastSelected == "ColorWheel");
  CHECK_FALSE(reply.has_value());
}

TEST_CASE("handleCommand dispatches set with the full payload") {
  FakeCommandHandler handler;
  handleCommand(R"({"action":"set","speed":20000})", handler);
  CHECK(handler.setCalls == 1);
  CHECK(handler.lastSetData["speed"] == 20000);
}

TEST_CASE("handleCommand dispatches options and returns a wrapped response") {
  FakeCommandHandler handler;
  handler.animationsToReturn = {"ColorWheel", "FadeColor"};
  auto reply = handleCommand(R"({"action":"options"})", handler);
  REQUIRE(reply.has_value());
  CHECK((*reply)["action"] == "options");
  CHECK((*reply)["animations"] == handler.animationsToReturn);
}

TEST_CASE("handleCommand ignores malformed JSON without crashing or calling the handler") {
  FakeCommandHandler handler;
  auto reply = handleCommand("not json at all {", handler);
  CHECK_FALSE(reply.has_value());
  CHECK(handler.selectCalls == 0);
  CHECK(handler.setCalls == 0);
  CHECK(handler.listCalls == 0);
}

TEST_CASE("handleCommand ignores an unknown action") {
  FakeCommandHandler handler;
  auto reply = handleCommand(R"({"action":"explode"})", handler);
  CHECK_FALSE(reply.has_value());
  CHECK(handler.selectCalls == 0);
}

// --- resolveAnimationPath() -------------------------------------------------

TEST_CASE("resolveAnimationPath resolves a bare name") {
  CHECK(resolveAnimationPath("./bin/animations", "ColorWheel") == "./bin/animations/ColorWheel.so");
}

TEST_CASE("resolveAnimationPath resolves a name with .so already appended") {
  CHECK(resolveAnimationPath("./bin/animations", "ColorWheel.so") == "./bin/animations/ColorWheel.so");
}

TEST_CASE("resolveAnimationPath resolves the historical full path the same way") {
  CHECK(resolveAnimationPath("./bin/animations", "./bin/animations/ColorWheel.so") == "./bin/animations/ColorWheel.so");
}

TEST_CASE("resolveAnimationPath never escapes the animations directory") {
  std::string path = resolveAnimationPath("./bin/animations", "../../../../etc/passwd");
  CHECK(path == "./bin/animations/passwd.so");
}

TEST_CASE("resolveAnimationPath rejects an empty name") {
  CHECK(resolveAnimationPath("./bin/animations", "") == "");
}

// --- AnimationCommandHandler -------------------------------------------------

TEST_CASE("AnimationCommandHandler::selectAnimation updates the shared selection") {
  AniManager manager(nullptr);
  std::string selected = "./bin/animations/Text.so";
  AnimationCommandHandler handler(manager, selected, "./bin/animations");

  handler.selectAnimation("ColorWheel");
  CHECK(selected == "./bin/animations/ColorWheel.so");
}

TEST_CASE("AnimationCommandHandler::selectAnimation with a bogus name leaves the current selection untouched") {
  AniManager manager(nullptr);
  std::string selected = "./bin/animations/Text.so";
  AnimationCommandHandler handler(manager, selected, "./bin/animations");

  handler.selectAnimation("");
  CHECK(selected == "./bin/animations/Text.so");
}

TEST_CASE("AnimationCommandHandler::setParams before anything has loaded does not crash") {
  AniManager manager(nullptr);
  std::string selected = "./bin/animations/Text.so";
  AnimationCommandHandler handler(manager, selected, "./bin/animations");

  handler.setParams(json{{"speed", 123}});
  CHECK_FALSE(manager.isReady());
}

TEST_CASE("AnimationCommandHandler::listAnimations returns bare names, not full paths") {
  std::filesystem::path dir = std::filesystem::temp_directory_path() / "cube_test_animations";
  std::filesystem::create_directories(dir);
  std::ofstream(dir / "ColorWheel.so").close();
  std::ofstream(dir / "FadeColor.so").close();

  AniManager manager(nullptr);
  std::string selected;
  AnimationCommandHandler handler(manager, selected, dir.string());

  std::vector<std::string> names = handler.listAnimations();
  std::sort(names.begin(), names.end());
  CHECK(names == std::vector<std::string>{"ColorWheel", "FadeColor"});

  std::filesystem::remove_all(dir);
}
