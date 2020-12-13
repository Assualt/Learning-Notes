CC=g++
CFLAGS=-I../log -I../http -I../http/url/include -I../hash -I../argparse -Wno-deprecated-declarations -std=c++11 -DUSE_OPENSSL -g #-O2

vpath %.cpp ../log ../hash

URLPARSE=-L ../http/url -lurlparse
GURLLIB=-L ../http/gurl -gurl


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

