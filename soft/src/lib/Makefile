INC=-I../ \
    -I./
CFLAGS=$(INC) -std=c99
CC=gcc
LD=gcc


all:clean fifo.o test.o
	$(LD) test.o fifo.o -o test

test.o: test.c
	$(CC) $(CFLAGS) -c test.c -o test.o

fifo.o: fifo.c
	$(CC) $(CFLAGS) -c fifo.c -o fifo.o

clean:
	rm -rf *.o *.exe
