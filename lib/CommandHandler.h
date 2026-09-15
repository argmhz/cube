#pragma once

#include <optional>
#include <string>
#include <vector>
#include "json.hpp"
#include "Log.h"

using json = nlohmann::json;

// Pure protocol layer: what a command *means*, decoupled from how its bytes
// arrived (a live TCP socket, a test, anything). No sockets, no hardware.
class CommandHandler {
public:
  virtual void selectAnimation(const std::string &name) = 0;
  virtual void setParams(const json &data) = 0;
  virtual std::vector<std::string> listAnimations() = 0;
  virtual ~CommandHandler() = default;
};

// Parses one raw message and dispatches it to `handler`. Malformed JSON is
// swallowed (same behaviour as before), never thrown further. Returns a
// response to send back to the caller only for "options"; nullopt otherwise.
inline std::optional<json> handleCommand(const std::string &raw, CommandHandler &handler) {
  try {
    json command = json::parse(raw);
    std::string action = command.value("action", "");
    Log::info("received action=" + action);

    if (action == "select") {
      handler.selectAnimation(command.value("animation", ""));
    } else if (action == "set") {
      handler.setParams(command);
    } else if (action == "options") {
      json result;
      result["action"] = "options";
      result["animations"] = handler.listAnimations();
      return result;
    }
  } catch (json::exception &e) {
    Log::warn(std::string("could not parse command: ") + e.what());
  }
  return std::nullopt;
}
