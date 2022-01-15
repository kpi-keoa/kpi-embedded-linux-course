CC := gcc
CFLAGS := -Wall -std=gnu11
DEPS := utils.h
OBJS := main.o utils.o

NAME := main

all: build

.PHONY: build clean

build: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	
