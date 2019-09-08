CC=gcc
CFLAGS= -lpthread
OBJECTS=pthreads_test2.c 
EXECUTABLE=hello

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@