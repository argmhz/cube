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
  void selectAnimation(const std::string &) override {}
  void setParams(const json &) override {}
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
