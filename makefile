#test:test.cpp
#	g++ -std=c++0x test.cpp -lboost_filesystem -lboost_system -lpthread -o test

#all: test P2Pserver server
#P2Pserver:P2Pserver.hpp
#	g++ -std=c++0x $@.hpp -lboost_filesystem -lboost_system -lpthread -o @^
#server:server.hpp
#	g++ -std=c++0x $@.hpp -lboost_filesystem -lboost_system -lpthread -o @^
#test:test.cpp
#	g++ -std=c++0x $@.cpp -lboost_filesystem -lboost_system -lpthread -o @^
all: main
main:main.cpp P2Pserver.hpp P2Pclient.hpp
		g++ -std=c++0x $^ -o $@ -lpthread -lboost_filesystem -lboost_system
#all: test test_cli
#test:test.cpp P2Pserver.hpp
#		g++ -std=c++0x $^ -o $@ -lpthread -lboost_filesystem -lboost_system
#test_cli:test_cli.cpp P2Pclient.hpp
#		g++ -std=c++0x $^ -o $@ -lpthread -lboost_filesystem -lboost_system
#all: P2Pserver
#P2Pserver:P2Pserver.hpp
#		g++ -std=c++0x P2Pserver.hpp  -o P2Pserver -lpthread -lboost_filesystem -lboost_system

 
