
CFLAGS = -Wall -Wextra -std=c++11
LIBS = -lboost_system -lboost_filesystem -lcrypto
dupefind: dupefind.cpp
	g++ $(CFLAGS) dupefind.cpp -o dupefind $(LIBS)
debug: dupefind.cpp
	g++ $(CFLAGS) -DDEBUG dupefind.cpp -o dupefind $(LIBS)
