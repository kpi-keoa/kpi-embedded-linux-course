TARGET = lab1_threaded_applications

CC = gcc

CFLAGS = -lpthread

SRC = src/lab1_threaded_applications.c

all:	$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(SRC) $(CFLAGS) -o $@ 

optimization: o0 o2

o0: $(OBJ)
	$(CC) $(SRC) -o0 $(CFLAGS) $@ 

o2: $(OBJ)
	$(CC) $(SRC) -o2 $(CFLAGS) $@ 
	
.c.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	@echo Cleaning...
	rm -f src/$(TARGET)
