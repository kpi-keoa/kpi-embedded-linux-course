CC = gcc
CFILES = src/thread.c

CFLAGS = -std=gnu11 -D_REENTRANT -lpthread

CFLAG_O0 = -O0
CFLAG_O2 = -O2

NAME1 = thread0
NAME2 = thread2

all: clean build run

clean:
	rm -rf $(NAME1) $(NAME2) *.txt

build: clean
	$(CC) $(CFILES) $(CFLAG_O0) $(CFLAGS) -o $(NAME1)
	$(CC) $(CFILES) $(CFLAG_O2) $(CFLAGS) -o $(NAME2)

run:
	./$(NAME1)

runO2:
	./$(NAME2)

