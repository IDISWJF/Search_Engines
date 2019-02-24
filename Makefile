FLAG=-std=c++11 -lboost_filesystem -lboost_system -ljsoncpp 
.PHONY:all
all:test libsearcher.so
test:test.cc searcher.hpp searcher.cc  		
	g++ test.cc searcher.cc -o test $(FLAG)
libsearcher.so:searcher.hpp searcher.cc 
	g++ searcher.cc -shared -fPIC -o libsearcher.so $(FLAG)
