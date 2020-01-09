CC = gcc

CFILES = src/thread1.c

CFLAGS = -std=c11 -D_REENTRANT -lpthread
CFLAG_0 = -o0
SFLAG_2 = -o2

NAME0 = thread0
NAME2 = thread2

all: install run

install:
	@$(CC) $(CFILES) $(CFLAG_0) $(CFLAGS) -o $(NAME0)
	@$(CC) $(CFILES) $(CFLAG_2) $(CFLAGS) -o $(NAME2)

run:
	@echo O0 optimization
	@./$(NAME0)
	@echo O2 optimization
	@./$(NAME2)

