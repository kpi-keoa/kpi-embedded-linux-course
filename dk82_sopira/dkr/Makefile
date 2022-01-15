CC := gcc
CFLAGS := -Wall -std=gnu11
DEPS := rand.h utils.h
OBJS := main.o rand.o utils.o

PROJ_NAME := main

all: build

.PHONY: build clean delsav

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	
delsav:
	rm -f data.sav
	
