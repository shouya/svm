

all:
	gcc -c svm.c -o svm.o -Wall -g
	gcc -c inst.c -o inst.o -Wall -g
	gcc -o svm svm.o inst.o -Wall -g

clean:
	rm -f svm.o inst.o svm