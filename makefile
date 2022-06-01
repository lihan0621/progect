.PHONY: cloud
cloud:cloud.cpp  util.hpp
	g++ -std=c++14 $^ -o $@ -L./lib -lstdc++fs -ljsoncpp -lbundle -lpthread
