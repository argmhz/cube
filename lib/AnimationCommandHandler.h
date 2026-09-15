#pragma once

#include <filesystem>
#include <string>
#include "CommandHandler.h"
#include "AniManager.cpp"

// Extracts just the animation name from whatever the client sent -- a bare
// name ("ColorWheel"), "ColorWheel.so", or the historical full path
// ("./bin/animations/ColorWheel.so") all resolve to the same, safe result.
// Directory components and any ".." are discarded by std::filesystem::path,
// so the resolved path can never leave `animationsDir`, regardless of what
// a client sends (e.g. a path traversal attempt like "../../etc/passwd"
// resolves to "<animationsDir>/passwd.so", never to the real /etc/passwd).
inline std::string resolveAnimationPath(const std::string &animationsDir, const std::string &clientInput) {
  std::string name = std::filesystem::path(clientInput).stem().string();
  if (name.empty()) {
    return "";
  }
  return (std::filesystem::path(animationsDir) / (name + ".so")).string();
}

// The real CommandHandler used by apps/socket.cpp: wraps AniManager (which
// has no hardware dependency itself, so this whole class is unit-testable).
class AnimationCommandHandler : public CommandHandler {
  AniManager &manager;
  std::string &selectedAnimation;
  std::string animationsDir;

public:
  AnimationCommandHandler(AniManager &manager, std::string &selectedAnimation, std::string animationsDir)
    : manager(manager), selectedAnimation(selectedAnimation), animationsDir(animationsDir) {}

  void selectAnimation(const std::string &name) override {
    std::string path = resolveAnimationPath(animationsDir, name);
    if (path.empty()) {
      return;
    }
    selectedAnimation = path;
    manager.stopAnimation();
  }

  void setParams(const json &data) override {
    if (manager.isReady()) {
      manager.getAnimation().onDataUpdate(data);
    }
  }

  std::vector<std::string> listAnimations() override {
    std::vector<std::string> files = manager.getAnimationsFiles(animationsDir);
    std::vector<std::string> names;
    for (const auto &file : files) {
      names.push_back(std::filesystem::path(file).stem().string());
    }
    return names;
  }
};
