.PHONY: clean

CFLAGS = -ggdb -Wall -std=gnu17
CC = gcc
SOURCES = $(shell echo src/*.c)
HEADERS = $(shell echo inc/*.h)
TARGET = db

all: build

build:
	$(CC) ${CFLAGS} $(SOURCES) -o $(TARGET)

clean:
	rm db

