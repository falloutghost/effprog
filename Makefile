CC=gcc
CFLAGS=-g -O -Wall
LDFLAGS=-g

all: life

life: life.c hash_table.c
	$(CC) $(CFLAGS) -o life life.c hash_table.c

clean:
	rm -rf life *.o

coverage:
	$(CC) $(CFLAGS) --coverage -c -o life.o life.c
	$(CC) $(CFLAGS) --coverage -c -o hash_table.o hash_table.c
	$(CC) $(LDFLAGS) -lgcov --coverage life.o hash_table.o -o life
