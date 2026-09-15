
animationsPaths = $(wildcard animations/*.cpp)
appsPaths = $(wildcard apps/*.cpp)

animations = $(subst animations/,,$(subst .cpp,,$(animationsPaths)))
apps = $(subst apps/,,$(subst .cpp,,$(appsPaths)))

build: $(animations) $(apps)

$(animations):
	g++ -fPIC -rdynamic -shared -o bin/animations/$@.so animations/$@.cpp -std=c++17

$(apps):
	g++ -W -o ./bin/$@ ./apps/$@.cpp -ldl -lbcm2835 -pthread -std=c++17 -lstdc++fs

testPaths = $(wildcard tests/*.cpp)

# Hardware-free unit tests: no bcm2835, no root, no Raspberry Pi required.
# (named "check", not "test", since "test" is already an app built from apps/test.cpp)
check:
	mkdir -p bin
	g++ -std=c++17 -Ilib -o bin/test_runner $(testPaths) lib/CubeBuffer.cpp lib/Socket.cpp -pthread -ldl -lstdc++fs
	./bin/test_runner
