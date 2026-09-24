#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include "Socket.h"
#include "../core/CubeBuffer.h"
#include "../Log.h"

// Streams what the cube is currently displaying to any connected client as
// newline-delimited JSON -- one line per frame, in exactly the format
// sim/simulator.cpp records:
//
//   {"t":<ms since start>,"voxels":[[r,g,b] x512]}
//
// sim/viewer replays a .jsonl file with the same parser, so a live feed and
// a recording are the same thing to it.
//
// This exists so the whole stack -- browser, cube-client, socket server,
// animation -- can be run and watched on a normal machine (see `make
// sim-socket`). It is off unless --stream-port is given, so on the Pi
// nothing listens and no frame is ever serialized. It only ever reads the
// voxel buffer, so it cannot disturb what the animation is drawing.

// Voxel order is z outer, y middle, x inner -- the order sim/viewer builds
// its positions in. Changing it here silently scrambles the viewer.
inline std::string serializeFrame(CubeBuffer &cube, long ms) {
  std::string out = "{\"t\":" + std::to_string(ms) + ",\"voxels\":[";
  for (int z = 0; z < 8; z++) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        CubeBuffer::Color c = cube.get(x, y, z);
        if (z || y || x) {
          out += ",";
        }
        out += "[" + std::to_string(c.red) + "," + std::to_string(c.green) + "," + std::to_string(c.blue) + "]";
      }
    }
  }
  out += "]}\n";
  return out;
}

// send() until the whole frame is gone or the peer breaks. Socket::socket_write
// makes a single send() call and reports success on a partial write, which
// for a ~4 KB frame would hand the viewer half a JSON line. MSG_NOSIGNAL
// keeps a viewer that closes its tab from killing the process with SIGPIPE.
inline bool writeFrame(int fd, const std::string &frame) {
  size_t sent = 0;
  while (sent < frame.size()) {
    ssize_t n = ::send(fd, frame.data() + sent, frame.size() - sent, MSG_NOSIGNAL);
    if (n <= 0) {
      return false;
    }
    sent += (size_t)n;
  }
  return true;
}

// Accepts viewers forever on its own thread. Blocking accept() is fine here
// precisely because it is its own thread -- Socket::select only has
// one-second granularity, too coarse to share a thread with a 30fps loop.
inline void acceptViewers(Socket *listener, std::vector<int> *clients, std::mutex *clientsMutex) {
  while (true) {
    Socket *viewer = listener->accept();
    if (viewer->sock < 0) {
      delete viewer;
      continue;
    }
    Log::info("frame viewer connected from " + viewer->address);
    {
      std::lock_guard<std::mutex> lock(*clientsMutex);
      clients->push_back(viewer->sock);
    }
    // Only the fd is kept; the wrapper has served its purpose.
    delete viewer;
  }
}

inline void runFrameStream(CubeBuffer &cube, const std::string &host, const std::string &port, int fps) {
  Socket *listener = new Socket(AF_INET, SOCK_STREAM, 0);
  int optVal = 1;
  listener->socket_set_opt(SOL_SOCKET, SO_REUSEADDR, &optVal);
  if (listener->bind(host, port) < 0 || listener->listen(4) < 0) {
    Log::warn("frame stream could not listen on " + host + ":" + port);
    return;
  }
  Log::info("streaming frames on " + host + ":" + port);

  std::vector<int> clients;
  std::mutex clientsMutex;
  std::thread(acceptViewers, listener, &clients, &clientsMutex).detach();

  auto start = std::chrono::steady_clock::now();
  auto interval = std::chrono::milliseconds(1000 / (fps > 0 ? fps : 30));

  while (true) {
    std::this_thread::sleep_for(interval);

    {
      std::lock_guard<std::mutex> lock(clientsMutex);
      if (clients.empty()) {
        // Nobody watching: skip the ~4 KB of serialization entirely.
        continue;
      }
    }

    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
    std::string frame = serializeFrame(cube, ms);

    std::lock_guard<std::mutex> lock(clientsMutex);
    for (size_t i = 0; i < clients.size();) {
      if (writeFrame(clients[i], frame)) {
        i++;
      } else {
        Log::info("frame viewer disconnected");
        ::close(clients[i]);
        clients.erase(clients.begin() + i);
      }
    }
  }
}
