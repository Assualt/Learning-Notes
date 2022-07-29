CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g -I../threadpool -I../conf

vpath %.cpp ../log ../hash ../conf


ALLOBJS=log.o  mailserver.o unitMailServer.o mailenv.o configure.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: mailserver

mailserver: $(ALLOBJS)
	$(CC) $^ -o mailserver $(CFLAGS) -lz -g -std=c++11 -pthread -lmagic

clean:
	rm -rf *.o mailserver

