
animationsPaths = $(wildcard animations/*.cpp)
appsPaths = $(wildcard apps/*.cpp)

animations = $(subst animations/,,$(subst .cpp,,$(animationsPaths)))
apps = $(subst apps/,,$(subst .cpp,,$(appsPaths)))

build: $(animations) $(apps)

$(animations):
	g++ -fPIC -rdynamic -shared -o bin/animations/$@.so animations/$@.cpp -std=c++11

$(apps):
	g++ -W -o ./bin/$@ ./apps/$@.cpp -ldl -lbcm2835 -pthread -std=c++11
