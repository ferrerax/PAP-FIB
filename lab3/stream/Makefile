#CC = gcc-4.9
CC = gcc
CFLAGS = -Wall -O3 -fopenmp -DTUNED

all: stream

stream: stream.c
	$(CC) $(CFLAGS) stream.c -o stream -lm

clean:
	rm -f stream *.o

