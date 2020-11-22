TARGET = lab
SRCS := lab0.c

LIBS += -lm -pthread

CC = gcc
CFLAGS = -std=gnu18 -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic
CFLAGC += -Wint-conversion -Wreturn-type -Wincompatible-pointer-types
LDFLAGS = $(CFLAGS) -Wl,-as-needed $(LIBS)

OBJS := $(SRCS:.c=.o)

all: | $(TARGET) clean

clean:
	 rm -f $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@



.PHONY: clean all 
