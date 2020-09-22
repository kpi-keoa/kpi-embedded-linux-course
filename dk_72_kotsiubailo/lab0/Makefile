TARGET = pthreads
SRCS := threads.c


LIBS = pthread

CC = gcc
OPT_FLAG ?= -O0



CFLAGS = -march=x86-64 -mtune=generic $(OPT_FLAG) -Wall -Wextra -Wpedantic  
LDFLAGS = $(CFLAGS) -Wl,-as-needed $(addprefix -,$(LIBS))
ASMFLAGS = $(LDFLAGS) -fverbose-asm -S

OBJS := $(SRCS:.c=.o)
ASM  := $(SRCS:.c =.S)

BUILD_DIR  = ./build
SOURCE_DIR = ./src

$(BUILD_DIR):
	    mkdir -p $@


.PHONY: all
all: | $(BUILD_DIR) clean $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(ASM)

.PHONY: target
target: | $(BUILD_DIR) clean $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/$(OBJS)  
	    $(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/$(ASM): $(SOURCE_DIR)/$(SRCS) 
	    $(CC) $(ASMFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	    $(CC) $(CFLAGS) -c $^ -o $@

.PHONY: asm 
asm: | $(BUILD_DIR) clean $(BUILD_DIR)/$(ASM)

.PHONY: clean
clean: 
	    rm -f $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJS) $(BUILD_DIR)/$(ASM)

.PHONY: delete_dir
 delete_dir: rm -rf $(BUILD_DIR)
    
