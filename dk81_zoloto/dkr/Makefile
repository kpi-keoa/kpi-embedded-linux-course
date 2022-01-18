all: find-new.o utils.o
	gcc find-new.o utils.o -o find-new

build : all

find-new.o: find-new.c
	gcc -c -Wall -Wextra find-new.c

utils.o: utils.c
	gcc -c -Wall -Wextra utils.c

.PHONY: clean

clean:
	rm -rf *.o *.out main
