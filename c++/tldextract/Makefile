CC=g++
CFLAGS=-I../log -I../http -I../http/url/include -I../hash -I../argparse -Wno-deprecated-declarations -std=c++11 -DUSE_OPENSSL

vpath %.cpp ../log

URLPARSE=-L ../http/url -lurlparse
GURLLIB=-L ../http/gurl -gurl


ALLOBJS=logging.o tldextract.o unitTldExtract.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: tld-url


tld-gurl: $(ALLOBJS)
	$(CC) $^ -o tldextract $(CFLAGS) -I./gurl -L./gurl/ -lgurl -lz -g -DUSE_GURL -lssl -lcrypto -pthread $(GURLLIB)

tld-url: $(ALLOBJS)
	$(CC) $^ -o tldextract $(CFLAGS) -lz -g  -lssl -lcrypto -pthread $(URLPARSE)

zlib: logging.o test_gzip.o
	$(CC) $^ -o zlib  -lz -g $(CFLAGS)

clean:
	rm -rf *.o zlib tldextract
