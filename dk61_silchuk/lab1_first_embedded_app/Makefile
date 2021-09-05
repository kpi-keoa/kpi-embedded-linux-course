# Executable-target files, source files, object files, headers to include
TARGET=thrds_app
SRC=threads
SRC_PATH=src/
SRC:=$(addsuffix .c, $(SRC))
SRC:=$(addprefix $(SRC_PATH), $(SRC))
HEADERS=

# Compiler settings
CC=gcc	
CFLAGS=-Wall -Wextra -Wpedantic
LIBS=-lpthread
OPTFLAG=

.PHONY: all clean help

help:	
	@echo -e "Available commands: all, asm, clean, help.\
		\n\nUse:\n\tmake all [OPTFLAG=...]\tto build the project.\
		\n\n\tmake asm [OPTFLAG=...]\tto get ASM code.\
		\n\nOPTFLAG can be: =-O1, =-O2, =-O3, =-Os, =-Ofast."

all: clean $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(OPTFLAG) $(CFLAGS) $(LIBS) $(SRC) -o $@

asm: $(SRC) $(HEADERS)
	$(CC) $(OPTFLAG) $(CFLAGS) $(LIBS) $(SRC) -S 

clean:
	@echo "Cleaning..."
	rm -f *.s $(TARGET)

