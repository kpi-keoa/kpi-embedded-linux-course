// most code stolen from lecture example
TARGET ?= thread
SRC := src/thread.c
CC = gcc
OPTFLAG ?= -O2
CFLAGS = -Wall -Wpedantic
LDFLAGS := -pthread

#gcc -O2 -Wall thread.c -pthread -o ./thread

.PHONY: all clean tidy asm

all: $(TARGET)

$(TARGET): $(TARGET)

$(TARGET): $(SRC:.c=.o)
	$(CC) $(OPTFLAG) $(CFLAGS) $(LDFLAGS) $^ -o $@
	
%.o: %.c
	$(CC) $(OPTFLAG) $(CFLAGS) -c $< -o $@

# remove build artifacts
clean:
	@echo Cleaning up
	rm -f $(SRC:.c=.o)
	rm -f $(TARGET).S

# remove build artifacts & target
tidy: clean
	rm -f $(TARGET)
	
asm: 
	$(CC) $(OPTFLAG) $(CFLAGS) $(LDFLAGS) -fverbose-asm -S $(SRC) -o $(TARGET).S

