CC=gcc
CFLAGS=-std=gnu11 -lpthread -O2
OBJECTS=patch.o
EXECUTABLE=hello

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(EXECUTABLE) $(OBJECTS)