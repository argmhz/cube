#pragma once

#include <string>
#include <vector>

// Runtime configuration for apps/socket.cpp, with the same defaults it
// always used, now overridable instead of hardcoded.
struct Config {
  std::string host = "localhost";
  std::string port = "1234";
  std::string animationsDir = "./bin/animations";
};

// Parses --host/--port/--animations-dir (each takes one value). Deliberately
// simple: unknown flags are ignored, and a flag with no following value is
// ignored rather than crashing -- not a full CLI parser, just enough to get
// the previously-hardcoded values out of the source.
inline Config parseArgs(const std::vector<std::string> &args) {
  Config config;
  for (size_t i = 0; i < args.size(); i++) {
    if (args[i] == "--host" && i + 1 < args.size()) {
      config.host = args[++i];
    } else if (args[i] == "--port" && i + 1 < args.size()) {
      config.port = args[++i];
    } else if (args[i] == "--animations-dir" && i + 1 < args.size()) {
      config.animationsDir = args[++i];
    }
  }
  return config;
}
