
.PHONY: build

all: build clean

build: src/thread.c src/func.c
	gcc -o main -lpthread -O0 src/thread.c src/func.c
	./main
	gcc -o main -lpthread -O2 src/thread.c src/func.c
	./main

clean:
	rm -rf *.o main

help:
	@echo 'list of comands for make: all, build, clean'