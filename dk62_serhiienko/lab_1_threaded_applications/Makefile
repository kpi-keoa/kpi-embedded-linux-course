CC = gcc
CC_ARM = arm-linux-gnueabihf-gcc
CFLAGS =-pthread -O2
DEPS = src/thread
OUT = test.out
all: clean build
clean:
	rm -f $(OUT) *.o
build:
	$(CC) $(CFLAGS) $(addsuffix .c, $(DEPS)) -o $(OUT)

build-arm:
	$(CC_ARM) $(CFLAGS) $(addsuffix .c, $(DEPS)) -o $(OUT)
