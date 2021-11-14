CC=g++
CFLAGS=-I../log -I../http -I../http/url/include -I../hash -I../argparse -I../json -Wno-deprecated-declarations -std=c++11 -DUSE_OPENSSL -g

vpath %.cpp ../log ../json

URLPARSE=-L ../http/url -lurlparse
GURLLIB=-L ../http/gurl -gurl


ALLOBJS=logging.o unitTicketQuery.o json.o

.cpp.o:
	$(CC) -c $(CFLAGS) $< 


all: ticket-url


ticket-gurl: $(ALLOBJS)
	$(CC) $^ -o ticket $(CFLAGS) -I./gurl -L./gurl/ -lgurl -lz -g -lssl -lcrypto -pthread $(GURLLIB)

ticket-url: $(ALLOBJS)
	$(CC) $^ -o ticket $(CFLAGS) -lz -g -std=c++11 -lssl -lcrypto -pthread $(URLPARSE)

clean:
	rm -rf *.o ticket

