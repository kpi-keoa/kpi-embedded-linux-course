TARGET=emb_uthreads
CC=gcc
CFLAGS=-O2 -Wall -Wextra -Wpedantic -Werror -std=gnu11 -pthread

.PHONY: all clean

help:				
	@echo Available options:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

all: clean | $(TARGET) 		## assembly 

$(TARGET):
	$(CC) $(CFLAGS) src/$(addsuffix .c,$(TARGET)) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:	 			## clean from binaries
	-rm -f src/*.o $(TARGET)
