CC  = g++
CXX = gcc
FLAG = -Wno-literal-suffix -Wno-deprecated -Wsign-compare  -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_THREAD_PRIORITY_SCHEDULING
CFLAGS 	= $(FLAG)  -g -Wall -I./include  $(SYSFLAGS) -std=c++11 -O3
#VPATH
VPATH = src:src/base:test:src/conf
#LIBS

BASEOBJS = xstring.o xstringbuilder.o xtoken.o xconfmgrbase.o\

ALLOBJS =  $(BASEOBJS)

libxmt.a: $(ALLOBJS)
	rm -f libxmt.a
	ar cr libxmt.a $(ALLOBJS)
	ranlib libxmt.a
.cpp.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
.cc.o:
	$(CC) -c $(CFLAGS) $(OPT) $<
all:libxmt.a
	
clean:
	rm -rf *.o *.a *.lo *.d
r: clean all

# TEST
test_tstring: test_tstring.o  $(ALLOBJS)
	$(CC) $(CFLAGS) -o teststring test_tstring.o $(ALLOBJS) 
cleanTest:
	rm -rf test_tstring.o test test_tstring.d

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
