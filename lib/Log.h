#pragma once

#include <iostream>
#include <string>

// Minimal leveled logging to stderr. stderr is the right target here: it's
// already the pattern used elsewhere in this codebase (AniManager, Socket),
// and when running under systemd, stderr/stdout are captured automatically
// by the journal (`journalctl -u cube`) -- no log file management needed.
namespace Log {
  inline void info(const std::string &msg) {
    std::cerr << "[INFO] " << msg << std::endl;
  }
  inline void warn(const std::string &msg) {
    std::cerr << "[WARN] " << msg << std::endl;
  }
  inline void error(const std::string &msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
  }
}
