	g++  -fPIC -rdynamic -shared -o ./bin/animations/TestAni.so ./animations/TestAni.cpp -std=c++11
	g++ -W -o ./bin/Main  Animation.cpp test.cpp -ldl -std=c++11
