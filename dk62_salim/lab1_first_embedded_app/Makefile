
CFLAGS = -Wall -Wextra -Wpedantic -lpthread
SOURCES = thread.c

all: build run clean

build: $(SOURCES)
	gcc -o main_1 -O0 $(CFLAGS) $(SOURCES)
	gcc -o main_2 -O2 $(CFLAGS) $(SOURCES)

run: main_1 main_2
	./main_1
	./main_2

clean:
	rm -rf main_1 main_2

