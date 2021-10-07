CC = g++
CFLAGS =  -I../ -I../../ -g -std=c++11

vpath %.cpp ./: ../poller: ../

OBJS = Channel.o EventLoop.o Buffer.o Socket.o SocketsOp.o EPollPoller.o  PollPoller.o Poller.o CurrentThread.o

.cpp.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
.cc.o:
	$(CC) -c $(CFLAGS) $(OPT) $<

libnet.a: $(OBJS)
	rm -f libnet.a
	ar cr libnet.a $(OBJS)
	ranlib libnet.a

all: libnet.a

main: test_poller.o $(OBJS)
	$(CC) test_poller.o $(OBJS) -o $@ -L ../../base/test -lbase

clean:
	rm -rf *.o *.a main
 
