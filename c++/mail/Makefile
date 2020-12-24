CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g -I../threadpool

vpath %.cpp ../log ../hash


ALLOBJS=logging.o  mailserver.o unitMailServer.o mailconfig.o mailenv.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: mailserver

mailserver: $(ALLOBJS)
	$(CC) $^ -o mailserver $(CFLAGS) -lz -g -std=c++11 -pthread -lmagic

clean:
	rm -rf *.o mailserver

