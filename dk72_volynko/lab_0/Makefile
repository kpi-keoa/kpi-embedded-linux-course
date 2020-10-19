TARGET = lab0
SRCS := src/pthread.c

LIBS += -lm -pthread

CC = gcc
CFLAGS = -std=gnu18 -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic
LDFLAGS_O0 = $(CFLAGS) -O0 -Wl,-as-needed $(LIBS)
LDFLAGS_O2 = $(CFLAGS) -Wl,-as-needed $(LIBS)

OBJS0 := src/pthread_O0.o
OBJS2 := src/pthread_O2.o

all: | clean $(TARGET)_O2

clean:
	 rm -f $(TARGET)_O2  $(TARGET)_O0 $(OBJS0) $(OBJS2)

$(TARGET)_O0: $(OBJS0)
	$(CC) $(LDFLAGS_O0) $^ -o $@

$(TARGET)_O2: $(OBJS2)
	$(CC) $(LDFLAGS_O2) $^ -o $@

$(OBJS0): $(SRCS)
	$(CC) $(CFLAGS) -O0 -c $< -o $@

$(OBJS2): $(SRCS)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean all 
