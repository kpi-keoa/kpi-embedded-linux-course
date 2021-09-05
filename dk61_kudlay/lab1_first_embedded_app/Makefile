
# Made by CYB3RSP1D3R

CC = gcc
CFLAGS = -c -std=c11 -Wall -Werror -O0 -fverbose-asm -save-temps
INCLUDE_OBJ = -pthread
INCLUDE_SRC = -lpthread
SRC = main.c
OBJ = main.o
EXEC = 2_cores_ctr

.PHONY: clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(INCLUDE_OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(INCLUDE_SRC)

clean:
	rm -f $(EXEC) $(OBJ) *.s *.i

