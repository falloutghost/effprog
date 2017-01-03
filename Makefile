CC=gcc
CFLAGS=-g -O2 -Wall -std=gnu89 -DNDEBUG
LDFLAGS=-g

all: life-cell_table life-hash_table

life-cell_table: life-cell_table.c cell_table.c
	$(CC) $(CFLAGS) -o life-cell_table life-cell_table.c cell_table.c

life-hash_table: life-hash_table.c hash_table.c
	$(CC) $(CFLAGS) -o life-hash_table life-hash_table.c hash_table.c

clean:
	rm -rf life *.o *.gch *.gcno *.gcda

coverage:
	$(CC) $(CFLAGS) --coverage -c -o life.o life.c
	$(CC) $(CFLAGS) --coverage -c -o cell_table.o cell_table.c
	$(CC) $(LDFLAGS) -lgcov --coverage life.o cell_table.o -o life
