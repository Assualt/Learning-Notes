CC=g++

CFLAGS=-I../log -I../ -std=c++14

vpath %.cpp ../log ./tests

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: test_leveldb test_sqlite

test_leveldb: test_leveldbclient.o logging.o leveldbcli.o
	$(CC) $(CFLAGS) $^ -o $@ $(CFLAGS) -lleveldb

test_sqlite: test_sqliteclient.o logging.o sqlitecli.o
	$(CC) $(CFLAGS) $^ -o $@ $(CFLAGS) -lsqlite3

clean:
	rm -rf *.o test_leveldb test_sqlite