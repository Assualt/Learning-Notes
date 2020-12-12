CC=g++
CFLAGS=-I../log -I../argparse -Wno-deprecated-declarations -std=c++11  -g

vpath %.cpp ../log


ALLOBJS=logging.o base64.o  unitHash.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: hashutils

hashutils: $(ALLOBJS)
	$(CC) $^ -o hashutils $(CFLAGS) -lz -g -std=c++11

clean:
	rm -rf *.o hashutils

