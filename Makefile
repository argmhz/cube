
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

testPaths = $(wildcard tests/*.cpp)

# Hardware-free unit tests: no bcm2835, no root, no Raspberry Pi required.
# (named "check", not "test", since "test" is already an app built from apps/test.cpp)
check:
	mkdir -p bin
	g++ -std=c++17 -Ilib -o bin/test_runner $(testPaths) lib/core/CubeBuffer.cpp lib/net/Socket.cpp -pthread -ldl -lstdc++fs
	./bin/test_runner
