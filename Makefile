
CFLAGS = -Wall -Wextra -std=c++11
LIBS = -lboost_system -lboost_filesystem -lcrypto

all: dupefind

debug: CFLAGS += -DDEBUG -g
debug: dupefind

dupefind: dupefind.cpp
	g++ $(CFLAGS) dupefind.cpp -o dupefind $(LIBS)
