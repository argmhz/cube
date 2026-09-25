#include "../lib/vendor/doctest.h"
#include "../lib/net/Socket.h"
#include <sys/socket.h>
#include <unistd.h>

// Socket::select() bug: the write-fd cleanup loop erased from `writes` using
// an iterator computed from `reads`. Exercised here with a real POSIX
// socketpair (portable, no Raspberry Pi/bcm2835 involved).

TEST_CASE("Socket::select filters reads and writes independently") {
  int fds[2];
  REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

  Socket a;
  a.sock = fds[0];

  std::vector<Socket> reads;
  reads.push_back(a);
  std::vector<Socket> writes;
  writes.push_back(a);

  Socket::select(&reads, &writes, NULL, 1);

  // Nothing has been written to fds[1], so fds[0] has nothing to read yet...
  CHECK(reads.empty());
  // ...but a fresh socket's send buffer is empty, so it must be writable.
  CHECK(writes.size() == 1);

  close(fds[0]);
  close(fds[1]);
}
