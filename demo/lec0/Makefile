TARGET = max
SRCS := good-max.c
# SRCS += ...
# SRCS += dep1.c other.c
LIBS = m

CC = gcc
CFLAGS = -march=x86-64 -mtune=generic -O2 -Wall -Wextra -Wpedantic    # -Werror
LDFLAGS = $(CFLAGS) -Wl,-as-needed $(addprefix -l,$(LIBS))

OBJS := $(SRCS:.c=.o)

all: | clean $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean all
