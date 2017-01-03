CC=gcc
CFLAGS=-g -Wall -O2 -DNDEBUG -std=gnu89 -m32
LDFLAGS=-g -m32

all: life-cell_table life-hash_table

life-hash_table: life-hash_table.c life.h hash_table.c hash_table.h
	$(CC) $(CFLAGS) -o life-hash_table life-hash_table.c hash_table.c

life-cell_table: life-cell_table.c life.h cell_table.c cell_table.h
	$(CC) $(CFLAGS) -o life-cell_table life-cell_table.c cell_table.c

clean:
	rm -rf life-hash_table life-cell_table *.o *.gch *.gcno *.gcda *.class

coverage: coverage-life-hash_table coverage-life-cell_table

coverage-life-hash_table: life-cell_table.c life.h cell_table.c cell_table.h
	$(CC) $(CFLAGS) --coverage -c -o life-hash_table.o life-hash_table.c
	$(CC) $(CFLAGS) --coverage -c -o hash_table.o hash_table.c
	$(CC) $(LDFLAGS) -lgcov --coverage life-hash_table.o hash_table.o -o life-hash_table

coverage-life-cell_table: life-cell_table.c life.h cell_table.c cell_table.h
	$(CC) $(CFLAGS) --coverage -c -o life-cell_table.o life-cell_table.c
	$(CC) $(CFLAGS) --coverage -c -o cell_table.o cell_table.c
	$(CC) $(LDFLAGS) -lgcov --coverage life-cell_table.o cell_table.o -o life-cell_table
