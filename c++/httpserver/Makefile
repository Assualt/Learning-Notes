CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g -I../threadpool

vpath %.cpp ../log ../hash


ALLOBJS=logging.o  httpserver.o unitHttpServer.o httputils.o httprequest.o httpresponse.o httpconfig.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: unitHttpServer

unitHttpServer: $(ALLOBJS)
	$(CC) $^ -o httpserver $(CFLAGS) -lz -g -std=c++11 -pthread -lmagic

clean:
	rm -rf *.o httpserver

