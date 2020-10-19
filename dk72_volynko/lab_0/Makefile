TARGET = lab0
SRCS := src/pthread.c

LIBS += -lm -pthread

CC = gcc
CFLAGS_O0 = -std=gnu18 -march=x86-64 -mtune=generic -O0 -Wall -Wextra -Wpedantic
CFLAGS_O2 = -std=gnu18 -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic
CFLAGC += -Wint-conversion -Wreturn-type -Wincompatible-pointer-types
LDFLAGS_O0 = $(CFLAGS_O0) -Wl,-as-needed $(LIBS)
LDFLAGS_O2 = $(CFLAGS_O2) -Wl,-as-needed $(LIBS)

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
	$(CC) $(CFLAGS_O0) -c $< -o $@

$(OBJS2): $(SRCS)
	$(CC) $(CFLAGS_O2) -c $< -o $@


.PHONY: clean all 
