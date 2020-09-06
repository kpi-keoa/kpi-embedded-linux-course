#Makefile for building code for avr microcontrollers

#------------------------------------------------------------------------------
# ------------------------YOU CAN(and NEED) TO CHANGE IT-----------------------
#------------------------------------------------------------------------------
TARGET = pthread
#------------------------------------------------------------------------------
	
#you need to contain all you *.h files in dir "inc" IT'S A RULE
INC_DIR = inc
#flags for compiler
WFLAGS = -Wall -Werror -Wpedantic
#level of optimisation
OPTIM = -O2
#standart
STD = -std=gnu18
#compiler
CC = gcc
#linker's flags
LDFLAGS = -pthread

#------------------------------------------------------------------------------
src_files := $(wildcard *.c)
obj_files := $(src_files:.c=.o)
asm_files := $(src_files:.c=.s)

#------------------------------------------------------------------------------
%.o: %.c
	$(CC) $(WFLAGS) $(OPTIM) $(STD) -I $(INC_DIR) -c $< -o $@

#------------------------------------------------------------------------------
%.s: %.c
	$(CC) $(WFLAGS) $(OPTIM) $(STD) -I $(INC_DIR) -fverbose-asm -S $< -o $@

#------------------------------------------------------------------------------
.PHONY: asm
asm: $(asm_files)

#------------------------------------------------------------------------------
.PHONY: build
build: $(obj_files)
	$(CC) $(LDFLAGS) -o $(TARGET) $^

#------------------------------------------------------------------------------
.PHONY: clean
clean:
	rm $(obj_files)
	@echo "Only <TARGET> left here!:)"

#------------------------------------------------------------------------------
.PHONY: tidy
tidy: clean
	rm $(TARGET)
	@echo "Your directory is tidy:)"
