CC=g++
CFLAGS=-I../log -I./ -I./url/include -I../hash -I../argparse -I../threadpool -Wno-deprecated-declarations -std=c++11 -DUSE_OPENSSL -g #-O2

vpath %.cpp ../log ../hash

URLPARSE=-L ./url -lurlparse
GURLLIB=-L ./gurl -gurl


ALLOBJS=logging.o unitHttpClient.o base64.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: http-url


http-gurl: $(ALLOBJS)
	$(CC) $^ -o httpclient $(CFLAGS) -I./gurl -L./gurl/ -lgurl -lz -g -lssl -lcrypto -pthread $(GURLLIB)

http-url: $(ALLOBJS)
	$(CC) $^ -o httpclient $(CFLAGS) -lz -g -std=c++11 -lssl -lcrypto -pthread $(URLPARSE)

clean:
	rm -rf *.o httpclient

