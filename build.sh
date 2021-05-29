	g++  -fPIC -rdynamic -shared -o ./bin/animations/TestAni.so ./animations/TestAni.cpp -std=c++11
	g++ -W -o ./bin/Main  Animation.cpp test.cpp -ldl -lbcm2835 -pthread -std=c++11
