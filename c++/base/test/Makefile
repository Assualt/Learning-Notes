

CC = g++
CFLAGS =  -I../ -g

vpath %.cpp  ../ : 

OBJS = Date.o File.o Format.o Logging.o Mutex.o Timestamp.o

.cpp.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
.cc.o:
	$(CC) -c $(CFLAGS) $(OPT) $<

libnet.a: $(OBJS)
	rm -f libbase.a
	ar cr libbase.a $(OBJS)
	ranlib libbase.a

all: libbase.a

test_timestamp: test_timestamp.cpp Timestamp.cpp
	$(CC) $(CFLAGS) $^ -o $@

test_logging: test_logging.cpp Logging.cpp
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -rf *.o *.a