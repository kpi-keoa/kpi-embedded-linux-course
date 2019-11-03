TARGETS = user
TARGETS1 = ioctl
CC = gcc
CFLAGS =-std=c11 -lpthread

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) -O0 $@.c -o $@-o

clean:
	rm -f *.i *.o *.s
