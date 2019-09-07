CC = gcc
CFILES = thread.c

CFLAGS1 = -std=c11 -O0 -D_REENTRANT -lpthread
CFLAGS2 = -std=c11 -O2 -D_REENTRANT -lpthread

NAME1 = thread1
NAME2 = thread2

all: install, clean
install:
	@$(CC) $(CFILES) $(CFLAGS1) -o $(NAME1)
	@$(CC) $(CFILES) $(CFLAGS2) -o $(NAME2)
clean:
	rm -f $(NAME1) $(NAME2)

