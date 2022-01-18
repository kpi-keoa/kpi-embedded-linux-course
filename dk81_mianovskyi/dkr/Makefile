.PHONY: clean

build:
	gcc -std=gnu18 -Wall -o game.o game.c utils.c my_rand.c -lm
	touch database.rst

clean:
	rm *.o
	rm database.rst
