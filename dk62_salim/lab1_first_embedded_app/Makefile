
CFLAGS = -Wall -Wextra -Wpedantic -lpthread
SOURCES = src/thread.c

all: | clean build run

build: $(SOURCES)
	gcc -o main_1 -O0 $(CFLAGS) $(SOURCES)
	gcc -o main_2 -O2 $(CFLAGS) $(SOURCES)

run: build
	./main_1
	./main_2

clean:
	rm -rf *.o main_1 main_2

help:
	@echo Commands that you can use:
	@echo make all - run all the commands below
	@echo make build - generate executable files 
	@echo make run - run executable files
	@echo make clean - clean all files after running make commands
	

