#include "../lib/vendor/doctest.h"
#include "../lib/net/ConnectionLoop.h"
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

// serveConnection() is the reconnect-critical piece: it must keep serving
// commands until the peer disconnects, then actually return (the old
// incoming() loop never did, which is why a dropped cube-client connection
// used to require restarting bin/socket by hand). Exercised with a real
// socketpair -- fully portable, no Raspberry Pi involved.

class RecordingCommandHandler : public CommandHandler {
public:
  int optionsCalls = 0;
  std::vector<json> params;
  void selectAnimation(const std::string &) override {}
  void setParams(const json &data) override { params.push_back(data); }
  std::vector<std::string> listAnimations() override {
    optionsCalls++;
    return {"ColorWheel"};
  }
};

TEST_CASE("serveConnection dispatches commands and returns once the peer disconnects") {
  int fds[2];
  REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

  Socket server;
  server.sock = fds[0];
  RecordingCommandHandler handler;

  std::thread serverThread([&]{
    serveConnection(server, handler);
  });

  // Client end: send one "options" command, then expect a reply.
  std::string request = R"({"action":"options"})";
  REQUIRE(write(fds[1], request.c_str(), request.size()) > 0);

  char buf[256] = {};
  ssize_t n = read(fds[1], buf, sizeof(buf) - 1);
  REQUIRE(n > 0);
  json reply = json::parse(std::string(buf, n));
  CHECK(reply["action"] == "options");
  CHECK(handler.optionsCalls == 1);

  // Simulate the client disconnecting -- serveConnection must return.
  close(fds[1]);
  serverThread.join();

  close(fds[0]);
}


// Framing: the mic capture in cube-client streams ~30 commands a second, so
// several land in one read and one can be split across two. Before
// drainCommands() every such read went to json::parse() whole and was
// dropped as malformed.

TEST_CASE("drainCommands splits several newline-delimited commands from one read") {
  RecordingCommandHandler handler;
  std::string pending = "{\"action\":\"set\",\"bands\":[1,2,3,4]}\n"
                        "{\"action\":\"set\",\"bands\":[5,6,7,8]}\n";

  drainCommands(pending, handler);

  REQUIRE(handler.params.size() == 2);
  CHECK(handler.params[0]["bands"][0] == 1);
  CHECK(handler.params[1]["bands"][0] == 5);
  CHECK(pending.empty());
}

TEST_CASE("drainCommands holds a partial command until the rest arrives") {
  RecordingCommandHandler handler;
  std::string pending = "{\"action\":\"set\",\"ba";

  drainCommands(pending, handler);
  CHECK(handler.params.empty());
  CHECK(pending == "{\"action\":\"set\",\"ba");

  pending += "nds\":[9,0,0,0]}\n";
  drainCommands(pending, handler);

  REQUIRE(handler.params.size() == 1);
  CHECK(handler.params[0]["bands"][0] == 9);
  CHECK(pending.empty());
}

TEST_CASE("drainCommands still accepts a bare command with no newline") {
  // The pre-framing cube-client sends one unterminated object per slider
  // move; it must keep working untouched.
  RecordingCommandHandler handler;
  std::string pending = R"({"action":"set","speed":20000})";

  drainCommands(pending, handler);

  REQUIRE(handler.params.size() == 1);
  CHECK(handler.params[0]["speed"] == 20000);
  CHECK(pending.empty());
}

TEST_CASE("drainCommands discards an unparseable backlog instead of growing forever") {
  RecordingCommandHandler handler;
  std::string pending(MAX_PENDING_BYTES + 1, 'x');

  drainCommands(pending, handler);

  CHECK(handler.params.empty());
  CHECK(pending.empty());
}
