#pragma once

#include <optional>
#include <string>
#include <vector>
#include "Socket.h"
#include "CommandHandler.h"

// Serves one already-accepted client connection: reads commands and
// dispatches them via `handler` until the peer disconnects or a read fails
// (both show up as socket_read() returning <= 0), then returns so the
// caller can go back to accepting a new connection.
//
// Fully portable (plain Socket usage, no hardware), so it can be exercised
// in tests with a real socketpair().
inline void serveConnection(Socket &client, CommandHandler &handler) {
  bool connected = true;
  while (connected) {
    std::vector<Socket> reads(1);
    reads[0] = client;
    if (Socket::select(&reads, NULL, NULL, 10) < 1) {
      continue;
    }

    std::string buffer;
    int bytesRead = client.socket_read(buffer, 1024);
    if (bytesRead <= 0) {
      connected = false;
      break;
    }

    std::optional<json> reply = handleCommand(buffer, handler);
    if (reply) {
      client.socket_write(reply->dump());
    }
  }
}
