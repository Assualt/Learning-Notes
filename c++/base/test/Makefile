CC = g++
CFLAGS =  -I../ -g

vpath %.cpp  ../ : ./


test_timestamp: test_timestamp.cpp Timestamp.cpp
	$(CC) $(CFLAGS) $^ -o $@

test_logging: test_logging.cpp Logging.cpp
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -rf *.o test_logging test_timestamp
