
TARGET = thread
SRCS := pthread_test.c

CC = gcc


CFLAGS := -march=x86-64 -mtune=generic -O0 -Wall -Wpedantic
LDFLAGS := $(CFLAGS) -pthread


OBJS := $(SRCS:.c=.o)
asm := $(SRCS:.c=.S)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.S: %.c
	$(CC) $(CFLAGS) -fverbose-asm -S $^ -o $@

.PHONY: all 
all: $(TARGET)


.PHONY: clean 
clean:
	rm -f $(TARGET) $(OBJS)


.PHONY: asm	
asm: 
	$(CC) $(CFLAGS) $(LDFLAGS) -fverbose-asm -S $(SRCS) -o $(TARGET).S
