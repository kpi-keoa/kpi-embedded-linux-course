TARGET = thread
SRCS := lab0.c
# SRCS += ...
# SRCS += dep1.c other.c
# LIBS = pthread

CC = gcc
CFLAGS = -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic -std=gnu18 -save-temps -fverbose-asm
LDFLAGS = $(CFLAGS) -Wl,-as-needed -pthread 


OBJS := $(SRCS:.c=.o)


all: | clean $(TARGET)


clean: 
	rm -f $(TARGET) $(OBJS) 

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean all
