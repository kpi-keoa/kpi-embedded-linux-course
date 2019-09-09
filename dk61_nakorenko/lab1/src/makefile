CC=gcc
CFLAGS= -lpthread -O0
OBJECTS=pthreads_test.c 
EXECUTABLE=hello

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm $(EXECUTABLE)
	