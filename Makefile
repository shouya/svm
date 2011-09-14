CC=gcc
CFLAGS=-O3 -Wall
DFLAGS=-g -Wall
OUT=svm

default: release

debug:
	$(CC) -c svm.c $(DFLAGS)
	$(CC) -c inst.c $(DFLAGS)
	$(CC) -o $(OUT) svm.o inst.o $(DFLAGS)

release:
	$(CC) -c svm.c $(CFLAGS)
	$(CC) -c inst.c $(CFLAGS)
	$(CC) -o $(OUT) svm.o inst.o $(CFLAGS)


all:
	gcc -c svm.c -o svm.o -Wall -g
	gcc -c inst.c -o inst.o -Wall -g
	gcc -o svm svm.o inst.o -Wall -g

clean:
	rm -f svm.o inst.o svm