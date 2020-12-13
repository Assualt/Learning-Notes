CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g

vpath %.cpp ../log ../hash


ALLOBJS=logging.o  httpserver.o unitHttpServer.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: unitHttpServer

unitHttpServer: $(ALLOBJS)
	$(CC) $^ -o httpserver $(CFLAGS) -lz -g -std=c++11 -pthread

clean:
	rm -rf *.o httpserver

