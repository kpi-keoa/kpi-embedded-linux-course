TARGET = thread_test
SRCS := pthread_test.c
LIBS = pthread
CC = gcc
CFLAGS = -march=x86-64 -mtune=generic -O0 -Wall -Wextra -Wpedantic -Werror
LDFLAGS = $(CFLAGS) $(addprefix -l,$(LIBS))
OBJS := $(SRCS:.c=.o)  
asm := $(SRCS:.c=.S)

all: | clean $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJS) $(TARGET).S

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.S: %.c
	$(CC) $(CFLAGS) -fverbose-asm -S $^ -o $@

asm: 
	$(CC) $(CFLAGS) $(LDFLAGS) -fverbose-asm -S $(SRCS) -o $(TARGET).S

<<<<<<< HEAD
.PHONY: clean all asm 
=======
.PHONY: clean all 
>>>>>>> d49dcc7431807823acdfc399c3be0dae17ffc4e9
