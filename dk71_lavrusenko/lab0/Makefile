TARGET = thread_task
SRCS := src/thread_task.c

LIBS += -lm -pthread

CC = gcc
CFLAGS = -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic
LDFLAGS = $(CFLAGS) -Wl,-as-needed $(LIBS)

OBJS := $(SRCS:.c=.o)

all: | clean $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean all
