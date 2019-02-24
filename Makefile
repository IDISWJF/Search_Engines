test:test.cc searcher.hpp searcher.cc  		
	g++ test.cc searcher.cc -o test -std=c++11 -lboost_filesystem -lboost_system
