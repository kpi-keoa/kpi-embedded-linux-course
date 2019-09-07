CC = gcc

CFILES = ./src/thread1.c

CFLAGS = -std=c11 -O0 -D_REENTRANT -lpthread

NAME = thread

all: install

install:
	@$(CC) $(CFILES) $(CFLAGS) -o $(NAME)

clean:	
	rm -rf thread