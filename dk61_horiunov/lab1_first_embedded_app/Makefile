CC = gcc
CFLAGS =   -lpthread
TARGET = src/test_lab1.c
RESULT_F = first_result
RESULT_S = second_result
.PHONY: clean build help all

all: build help

build:
	$(CC) -o $(RESULT_F) -O0 $(CFLAGS) $(TARGET)
	./$(RESULT_F)
	$(CC) -o $(RESULT_S) -O2 $(CFLAGS) $(TARGET)
	./$(RESULT_S)
clean:
	rm -rf *.o $(RESULT_S) $(RESULT_F)
help:
	 @echo 'For building  write: make build.For deleting write: make clean'
