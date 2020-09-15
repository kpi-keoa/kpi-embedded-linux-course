#Parts taken from Vadim Kharchuk code
TARGET = thread
SRC := src/thread.c
CC = gcc
OPTFLAG ?= -O2
CFLAGS = -Wall -Wpedantic
LDFLAGS := -pthread
OBJS := $(SRC:.c=.o)

.PHONY: all clean tidy asm

all: | clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OPTFLAG) $(CFLAGS) $(LDFLAGS) $^ -o $@
	
%.o: %.c
	$(CC) $(OPTFLAG) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC:.c=.o)
	rm -f $(TARGET).S

tidy:   clean
	rm -f $(TARGET)
	
asm: 
	$(CC) $(OPTFLAG) $(CFLAGS) $(LDFLAGS) -fverbose-asm -S $(SRC) -o $(TARGET).S
