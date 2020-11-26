all:
	g++ unitHttpClient.cpp logging.cpp -o httpclient -I./gurl -L./gurl/ -lgurl -I/home/cmdev/include -L/home/cmdev/lib -lz -g -std=c++11 -DUSE_OPENSSL -lssl -lcrypto -pthread

tld: 
	g++ unitTldExtract.cpp logging.cpp tldextract.cpp -o tldextract -I./gurl -L./gurl/ -lgurl -lz -g -std=c++11 -DUSE_OPENSSL -lssl -lcrypto -pthread

zlib: 
	g++ test_gzip.cpp logging.cpp -o zlib  -lz -g -std=c++11 
