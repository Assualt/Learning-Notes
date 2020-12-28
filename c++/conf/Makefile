CC=g++
CFLAGS=-I../log -I../argparse -I../hash -Wno-deprecated-declarations -std=c++11  -g

vpath %.cpp ../log 


ALLOBJS=logging.o  unitconf.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: conf

conf: $(ALLOBJS)
	$(CC) $^ -o unitconf $(CFLAGS) -lz -g -std=c++11 -pthread -lmagic

clean:
	rm -rf *.o unitconf

