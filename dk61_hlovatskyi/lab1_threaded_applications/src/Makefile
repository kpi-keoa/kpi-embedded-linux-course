TARGETS = threads

CC = gcc

CFLAGS = -g -save-temps -Wall -Wextra -Wpedantic -Werror -std=c99 -pthread 

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) -O0 $@.c -o $@O0.o
	$(CC) $(CFLAGS) -O2 $@.c -o $@O2.o
clean:
	rm -f *.i *.o *.s
