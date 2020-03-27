CC  = g++
CXX = gcc
FLAG = -Wno-literal-suffix -Wno-deprecated -Wsign-compare  -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_THREAD_PRIORITY_SCHEDULING
CFLAGS 	= $(FLAG)  -g -Wall -I../include  $(SYSFLAGS) -std=c++11 

#VPATH
VPATH = ../src:../src/base:../src/conf:../unittest/

#LIBS
TESTLIBS=-pthread -L/usr/local/lib/ -lgtest

BASEOBJS = xstring.o xstringbuilder.o xtoken.o xconfmgrbase.o

ALLOBJS =  $(BASEOBJS)

static_lib: $(ALLOBJS)
	ar cr libapp.a $(ALLOBJS)
	ranlib libapp.a
	install -d ../lib
	install -t ../lib libapp.a
	rm libapp.a -f

.cpp.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
.cc.o:
	$(CC) -c $(CFLAGS) $(OPT) $<

all:static_lib
	
clean:
	rm -rf *.o *.a *.lo *.d

r: clean all

EXECPATH="out"

# Unit_TEST
unit_string:unit_tstring.o
	$(CC) $(CFLAGS) unit_tstring.o -o unittstring $(TESTLIBS) -L../lib -lapp
unit_conf:unit_tconf.o
	$(CC) $(CFLAGS) unit_tconf.o -o unittconf $(TESTLIBS) -L../lib -lapp 

#case test 
test_app:testApp.o
	$(CC) $(CFLAGS) testApp.o -o testApp -L../lib -lapp

