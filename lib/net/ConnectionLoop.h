#pragma once

#include <optional>
#include <string>
#include <vector>
#include "Socket.h"
#include "CommandHandler.h"
#include "../Log.h"

// Upper bound on the unparsed tail we keep between reads. A client that
// never sends a newline and never forms valid JSON would otherwise grow
// this without limit; 64 KB is far above any real command (the largest,
// a Spectrum band update, is a couple of hundred bytes).
const size_t MAX_PENDING_BYTES = 64 * 1024;

// Pulls every complete command out of `pending`, dispatches it, and returns
// whatever replies the handler produced.
//
// Commands are newline-delimited. TCP is a byte stream with no message
// boundaries, so once a client streams commands continuously -- the mic
// capture in cube-client sends ~30 a second -- two of them routinely land
// in a single read, and a single one can just as easily be split across
// two. Both used to end up at json::parse() as one malformed string and be
// dropped with a warning; splitting on '\n' and keeping the remainder for
// the next read makes them work.
//
// A trailing fragment without a newline is held back, *unless* it already
// parses as a complete JSON value on its own. That keeps the older
// cube-client working unchanged: it sends one bare object per slider move
// and never terminates it.
inline std::vector<json> drainCommands(std::string &pending, CommandHandler &handler) {
  std::vector<json> replies;

  size_t newline;
  while ((newline = pending.find('\n')) != std::string::npos) {
    std::string message = pending.substr(0, newline);
    pending.erase(0, newline + 1);

    // Blank lines (an empty keep-alive, or \r\n) are not an error.
    if (message.find_first_not_of(" \t\r") == std::string::npos) {
      continue;
    }
    if (std::optional<json> reply = handleCommand(message, handler)) {
      replies.push_back(*reply);
    }
  }

  if (!pending.empty() && json::accept(pending)) {
    if (std::optional<json> reply = handleCommand(pending, handler)) {
      replies.push_back(*reply);
    }
    pending.clear();
  }

  if (pending.size() > MAX_PENDING_BYTES) {
    Log::warn("dropping " + std::to_string(pending.size()) + " bytes of unparseable input");
    pending.clear();
  }

  return replies;
}

// Serves one already-accepted client connection: reads commands and
// dispatches them via `handler` until the peer disconnects or a read fails
// (both show up as socket_read() returning <= 0), then returns so the
// caller can go back to accepting a new connection.
//
// Fully portable (plain Socket usage, no hardware), so it can be exercised
// in tests with a real socketpair().
inline void serveConnection(Socket &client, CommandHandler &handler) {
  bool connected = true;
  std::string pending;

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

    pending += buffer;
    for (const json &reply : drainCommands(pending, handler)) {
      client.socket_write(reply.dump());
    }
  }
}
