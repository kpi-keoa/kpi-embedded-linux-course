# Makefile for building application for lab1

CC = gcc
CFLAGS = -c -Wall -std=c99 -O0 -save-temps -Werror -lpthread
INCLUDE = -pthread
SOURCES	= main.c
OBJECTS	= main.o
EXECUTABLE = 2_cores_counter

all: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(INCLUDE)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean :
	rm -f $(EXECUTABLE) $(OBJECTS)
