# most code stolen from lecture example

TARGET = thread
SRCS := main.c
# SRCS += ...
# SRCS += dep1.c other.c
# LIBS = pthread

CC = gcc
CFLAGS = -march=x86-64 -mtune=generic -O0 -Wall -Wextra -Wpedantic -std=gnu18
LDFLAGS = $(CFLAGS) -Wl,-as-needed -pthread

OBJS := $(SRCS:.c=.o)

all: | clean $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean all
