# Makefile for building application for lab1
# Made by CYB3RSP1D3R

CC = gcc
CFLAGS = -c -Wall -std=c11 -O0 -Werror
SOURCES	= main.c
OBJECTS	= main.o
INCLUDE = -pthread
EXECUTABLE = 2_cores_counter

all: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(INCLUDE)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean :
	rm -f $(EXECUTABLE) $(OBJECTS)
