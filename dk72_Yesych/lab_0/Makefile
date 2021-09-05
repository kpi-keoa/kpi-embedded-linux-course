TARGET = threading
SRCS := pthread_test.c

LIBS = pthread

CC = gcc

VOLAT ?= 0
IN_DEF ?= VOLATILE=$(VOLAT)

OPT_FLAG ?= -O0

CFLAGS = $(addprefix -D,$(IN_DEF)) -march=x86-64 -mtune=generic $(OPT_FLAG) -Wall -Wextra -Wpedantic
LDFLAGS = $(CFLAGS) -Wl,-as-needed $(addprefix -,$(LIBS))
ASMFLAGS = $(LDFLAGS) -fverbose-asm -S

BUILD_DIR = ./build

SRC_DIR = ./src

$(BUILD_DIR):
	mkdir -p $@


OBJS := $(SRCS:.c=.o)
ASM  := $(SRCS:.c=.S)

all:    | $(BUILD_DIR) clean $(BUILD_DIR)/$(ASM) $(BUILD_DIR)/$(TARGET) 

asm:    | $(BUILD_DIR) clean $(BUILD_DIR)/$(ASM)

target: | $(BUILD_DIR) clean $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/$(OBJS)  
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/$(ASM): $(SRC_DIR)/$(SRCS) 
	$(CC) $(ASMFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	rm -f $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJS) $(BUILD_DIR)/$(ASM)

tidy :
	rm -rf $(BUILD_DIR)

.PHONY: clean all asm tidy target
