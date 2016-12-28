CC=gcc
CFLAGS=-g -O -Wall
LDFLAGS=-g
SOURCES=Makefile life.c life.h readlife.y showlife showlife.ps f0.l
GEN=f0500.l f1000.l f1500.l f2000.l f2500.l f3000.l

life: life.o readlife.tab.o

readlife.tab.o: readlife.tab.c

readlife.tab.c: readlife.y
	bison readlife.y

clean:
	rm -rf life *.o readlife.tab.c effizienz-aufgabe07

effizienz-aufgabe07.tar.gz: $(SOURCES) $(GEN)
	-rm -rf effizienz-aufgabe07
	mkdir effizienz-aufgabe07
	cp -p $(SOURCES) $(GEN) effizienz-aufgabe07
	tar cfz effizienz-aufgabe07.tar.gz effizienz-aufgabe07

dist: $(SOURCES) $(GEN) effizienz-aufgabe07.tar.gz
	cp -p $(SOURCES) $(GEN) effizienz-aufgabe07.tar.gz /nfs/unsafe/httpd/ftp/pub/anton/lvas/effizienz-aufgabe07