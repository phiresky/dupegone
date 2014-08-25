
CFLAGS = -Wall -Wextra -std=c++11
LIBS = -lboost_system -lboost_filesystem -lcrypto

all: dupegone

debug: CFLAGS += -DDEBUG -g
debug: dupegone

dupegone: dupegone.cpp
	g++ $(CFLAGS) dupegone.cpp -o dupegone $(LIBS)
