CC = gcc

CFILES = ./src/thread1.c

CFLAGS = -std=gnu11 -O0 -D_REENTRANT -lpthread

NAME = thread

all:
	@$(CC) $(CFILES) $(CFLAGS) -o $(NAME)

clean:	
	rm -rf thread