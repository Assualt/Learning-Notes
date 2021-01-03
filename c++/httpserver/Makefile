CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g -I../threadpool -I../conf -DUSE_OPENSSL

vpath %.cpp ../log ../hash ../conf


ALLOBJS=logging.o  httpserver.o unitHttpServer.o httputils.o httprequest.o httpresponse.o httpconfig.o configure.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: unitHttpServer

unitHttpServer: $(ALLOBJS)
	$(CC) $^ -o httpserver $(CFLAGS) -lz -g -std=c++11 -pthread -lmagic -lssl -lcrypto


clean:
	rm -rf *.o httpserver

