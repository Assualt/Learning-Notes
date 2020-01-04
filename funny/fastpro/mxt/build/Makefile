CC  = g++
CXX = gcc
FLAG = -Wno-literal-suffix -Wno-deprecated -Wsign-compare  -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_THREAD_PRIORITY_SCHEDULING
CFLAGS 	= $(FLAG)  -g -Wall -I../include  $(SYSFLAGS) -std=c++11 
#VPATH
VPATH = ../src:../src/base:../src/conf:../unittest/
#LIBS
TESTLIBS=-L /usr/local/lib/ -lgtest -pthread

BASEOBJS = xstring.o xstringbuilder.o xtoken.o xconfmgrbase.o

ALLOBJS =  $(BASEOBJS)

libxmt.a: $(ALLOBJS)
	rm ../lib/libxmt.a -f
	ar cr libxmt.a $(ALLOBJS)
	ranlib libxmt.a
	mv libxmt.a ../lib/ -f
.cpp.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
.cc.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
all:libxmt.a
	
clean:
	rm -rf *.o *.a *.lo *.d
r: clean all

EXECPATH="out"

# Unit_TEST
unit_string:unit_tstring.o
	$(CC) $(CFLAGS) unit_tstring.o -o unittstring $(TESTLIBS) -L../lib -lxmt


## Commands to generate dependency files
GEN_DEPS.c= $(CC) -E -MM $(DEFS) $(CFLAGS)
GEN_DEPS.cc= $(CXX) -E -MM $(DEFS) $(CFLAGS)
%.d : %.c
	@echo "generating dependency information for $<"
	@$(SHELL) -ec '$(GEN_DEPS.c) $< \
			| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
            [ -s $@ ] || rm -f $@'
%.d : %.cpp
	@echo "generating dependency information for $<"
	@$(SHELL) -ec '$(GEN_DEPS.cc) $< \
			| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
            [ -s $@ ] || rm -f $@'
OBJSDEP = $(ALLOBJS:.o=.d)
ifeq (,$(NAJECNDGLOBALS))
-include $(OBJSDEP)
else
ifneq ($(%clean,,$(MAKECMDGOBALS)),)
-include $(OBJSDEP)
endif
endif
