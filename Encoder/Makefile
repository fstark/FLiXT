CC=c++ -O3
CFLAGS=-Wall -g -std=c++17
LDFLAGS=-lpng

genvid: genvid.o
		$(CC) -o genvid genvid.o $(LDFLAGS)

genvid.o: genvid.cpp
		$(CC) -c genvid.cpp $(CFLAGS)

clean:
		rm -f genvid genvid.o

test:
		./genvid < matrix-90x43x256.txt
