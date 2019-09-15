TARGETS = src/main
CC = gcc
CFLAGS =-std=c99 -lpthread

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) -O0 $@.c -o $@-o0
	$(CC) $(CFLAGS) -O2 $@.c -o $@-o2

clean:
	rm -f *.i *.o *.s
