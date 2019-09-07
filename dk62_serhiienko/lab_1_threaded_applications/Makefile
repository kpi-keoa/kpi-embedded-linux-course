CC = gcc
CC_ARM = arm-linux-gnueabihf-gcc
CFLAGS_O2 = -O2 -pthread
CFLAGS_O0 = -O0 -pthread
DEPS = src/thread
OUT = test.out
all: clean build
clean:
	rm -f $(addprefix O2_, $(OUT)) $(addprefix O0_, $(OUT)) *.o
build:
	$(CC) $(CFLAGS_O2) $(addsuffix .c, $(DEPS)) -o $(addprefix O2_, $(OUT))
	$(CC) $(CFLAGS_O0) $(addsuffix .c, $(DEPS)) -o $(addprefix O0_, $(OUT))

build-arm:
	$(CC_ARM) $(CFLAGS_O2) $(addsuffix .c, $(DEPS)) -o $(addprefix O2_, $(OUT))
	$(CC_ARM) $(CFLAGS_O0) $(addsuffix .c, $(DEPS)) -o $(addprefix O0_, $(OUT))
    