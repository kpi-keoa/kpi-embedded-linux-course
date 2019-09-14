TARGETS = threads
CC = gcc
CFLAGS =-save-temps -Wall -Wextra -Wpedantic -Werror -std=c99 -D_REENTRANT -lpthread 

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) -O0 $@.c -o $@-o0
	$(CC) $(CFLAGS) -O2 $@.c -o $@-o2

clean:
	rm -f *.i *.o *.s
