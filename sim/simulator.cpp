#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include "../lib/core/Cube.h"
#include "../lib/animation/AniManager.h"

// Runs one real, unmodified animation .so (built for this machine against
// the fake bcm2835 -- see Makefile's `sim` target) and records what it
// draws to a .jsonl file: one line per frame, each a snapshot of all 512
// voxel colors. Meant to be played back by sim/viewer.html.

Cube * cube = new Cube;
AniManager * manager;

const std::string SIM_ANIMATIONS_DIR = "./bin/sim-animations";

void runAnimation(std::string soPath) {
  manager = new AniManager(cube);
  manager->loadAnimation(soPath.c_str());

  if (!manager->isReady()) {
    // loadAnimation() already printed *why* dlopen failed (bad path, wrong
    // arch, ...); calling getAnimation() here would dereference a null
    // Animation* and segfault instead of exiting cleanly.
    std::cerr << "Could not load animation from " << soPath << "\n";
    if (std::filesystem::exists(SIM_ANIMATIONS_DIR)) {
      std::cerr << "Available animations in " << SIM_ANIMATIONS_DIR << ":\n";
      for (const std::string &file : manager->getAnimationsFiles(SIM_ANIMATIONS_DIR)) {
        std::cerr << "  " << std::filesystem::path(file).stem().string() << "\n";
      }
    } else {
      std::cerr << SIM_ANIMATIONS_DIR << " does not exist -- did you run `make sim`?\n";
    }
    std::exit(1);
  }

  manager->getAnimation().draw(cube);
}

int main(int argc, char *argv[]) {
  std::string animationName;
  int seconds = 10;
  std::string outPath;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--animation" && i + 1 < argc) {
      animationName = argv[++i];
    } else if (arg == "--seconds" && i + 1 < argc) {
      seconds = std::atoi(argv[++i]);
    } else if (arg == "--out" && i + 1 < argc) {
      outPath = argv[++i];
    }
  }

  if (animationName.empty()) {
    std::cerr << "Usage: simulator --animation <Name> [--seconds N] [--out path.jsonl]\n";
    std::cerr << "<Name> must exist as bin/sim-animations/<Name>.so (built via `make sim`).\n";
    return 1;
  }

  std::string soPath = SIM_ANIMATIONS_DIR + "/" + animationName + ".so";

  std::ofstream outFile;
  std::ostream *out = &std::cout;
  if (!outPath.empty()) {
    outFile.open(outPath);
    out = &outFile;
  }

  std::thread cubeThread = cube->start();
  std::thread animationThread(runAnimation, soPath);

  auto start = std::chrono::steady_clock::now();
  auto frameInterval = std::chrono::milliseconds(33); // ~30fps

  while (std::chrono::steady_clock::now() - start < std::chrono::seconds(seconds)) {
    auto now = std::chrono::steady_clock::now();
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

    (*out) << "{\"t\":" << ms << ",\"voxels\":[";
    bool first = true;
    for (int z = 0; z < 8; z++) {
      for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
          Cube::Color c = cube->get(x, y, z);
          if (!first) (*out) << ",";
          first = false;
          (*out) << "[" << c.red << "," << c.green << "," << c.blue << "]";
        }
      }
    }
    (*out) << "]}\n";

    std::this_thread::sleep_for(frameInterval);
  }

  if (outFile.is_open()) {
    outFile.close();
  }

  std::cerr << "Captured " << seconds << "s to " << (outPath.empty() ? "stdout" : outPath) << "\n";
  // The animation's draw() and the cube's render thread both loop forever
  // by design (same as every real app in apps/) -- just exit the process
  // once we have our recording, rather than trying to join them.
  std::exit(0);
}
