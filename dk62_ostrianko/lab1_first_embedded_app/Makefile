
CFLAGS = -Iinc -lpthread
BUILDFILES = src/thread.c src/func.c
EXECUTABLE = main

.PHONY: clean

all: build clean

build:
	gcc -o $(EXECUTABLE) $(CFLAGS) -O0 $(BUILDFILES)
	./$(EXECUTABLE)
	gcc -o $(EXECUTABLE) $(CFLAGS) -O2 $(BUILDFILES)
	./$(EXECUTABLE)

clean:
	rm -rf *.o $(EXECUTABLE)

help:
	@echo 'list of comands for make: all, build, clean'