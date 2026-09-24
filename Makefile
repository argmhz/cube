
animationsPaths = $(wildcard animations/*.cpp)
appsPaths = $(wildcard apps/*.cpp)

animations = $(subst animations/,,$(subst .cpp,,$(animationsPaths)))
apps = $(subst apps/,,$(subst .cpp,,$(appsPaths)))

build: $(animations) $(apps)

# CubeBuffer.h/.cpp is a real header/implementation pair (unlike the rest of
# lib/, which is header-only) -- every animation and app compiles its own
# copy of it alongside its own source, in one g++ call, same as before.
CUBE_SRC = lib/core/CubeBuffer.cpp

$(animations):
	g++ -fPIC -rdynamic -shared -o bin/animations/$@.so animations/$@.cpp $(CUBE_SRC) -std=c++17

# apps/socket.cpp is the only app that needs Socket.cpp's implementation
# linked in (it's the only one that uses networking).
extra_socket = lib/net/Socket.cpp

$(apps):
	g++ -W -o ./bin/$@ ./apps/$@.cpp $(CUBE_SRC) $(extra_$@) -ldl -lbcm2835 -pthread -std=c++17 -lstdc++fs

# Simulator: builds the *real*, unmodified animation sources against a fake
# no-op bcm2835 (sim/fake_bcm2835/) instead of the real -lbcm2835, so they
# can run on a normal machine. Separate output dir (bin/sim-animations/)
# from the real ARM builds in bin/animations/ -- these .so files are for
# `bin/simulator` only, never meant to run on the actual Pi.
# -funsigned-char: plain `char` defaults to unsigned on ARM (the Pi) but
# signed on x86 -- lib/core/Cube.h relies on the unsigned default (e.g.
# `char layers[8] = {128,...}`), so we ask for it explicitly here.
sim: $(addprefix sim-,$(animations)) bin/simulator bin/sim-socket

sim-%:
	mkdir -p bin/sim-animations
	g++ -fPIC -shared -o bin/sim-animations/$*.so animations/$*.cpp $(CUBE_SRC) -Isim/fake_bcm2835 -funsigned-char -std=c++17

bin/simulator: sim/simulator.cpp
	mkdir -p bin
	g++ -W -o bin/simulator sim/simulator.cpp $(CUBE_SRC) sim/fake_bcm2835/fake_bcm2835.cpp -Isim/fake_bcm2835 -funsigned-char -ldl -pthread -std=c++17 -lstdc++fs

# The whole stack on a normal machine: the real, unmodified apps/socket.cpp
# built against the fake bcm2835, so cube-client can drive it over TCP 1234
# exactly as it drives the Pi, while --stream-port feeds sim/viewer a live
# picture of the cube. Run it with:
#
#   ./bin/sim-socket --animations-dir ./bin/sim-animations --stream-port 8421
#
# (An explicit rule beats the sim-% pattern rule above, so this target
# builds the server rather than being mistaken for an animation named
# "socket".)
sim-socket: bin/sim-socket $(addprefix sim-,$(animations))

bin/sim-socket: apps/socket.cpp $(CUBE_SRC) lib/net/Socket.cpp sim/fake_bcm2835/fake_bcm2835.cpp
	mkdir -p bin
	g++ -W -o bin/sim-socket apps/socket.cpp $(CUBE_SRC) lib/net/Socket.cpp sim/fake_bcm2835/fake_bcm2835.cpp -Isim/fake_bcm2835 -funsigned-char -ldl -pthread -std=c++17 -lstdc++fs

testPaths = $(wildcard tests/*.cpp)

# Hardware-free unit tests: no bcm2835, no root, no Raspberry Pi required.
# (named "check", not "test", since "test" is already an app built from apps/test.cpp)
check:
	mkdir -p bin
	g++ -std=c++17 -Ilib -Isim/fake_bcm2835 -funsigned-char -o bin/test_runner $(testPaths) lib/core/CubeBuffer.cpp lib/net/Socket.cpp sim/fake_bcm2835/fake_bcm2835.cpp -pthread -ldl -lstdc++fs
	./bin/test_runner
