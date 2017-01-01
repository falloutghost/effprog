CC=gcc
CFLAGS=-g -O2 -Wall -std=gnu89 -DNDEBUG
LDFLAGS=-g

all: life

life: life.c cell_table.c
	$(CC) $(CFLAGS) -o life life.c cell_table.c

clean:
	rm -rf life *.o *.gch *.gcno *.gcda

coverage:
	$(CC) $(CFLAGS) --coverage -c -o life.o life.c
	$(CC) $(CFLAGS) --coverage -c -o cell_table.o cell_table.c
	$(CC) $(LDFLAGS) -lgcov --coverage life.o cell_table.o -o life
