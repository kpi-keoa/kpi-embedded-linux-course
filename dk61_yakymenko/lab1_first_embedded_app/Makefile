CC=gcc
CFLAGS=-pthread

TARGET=thread_test
SOURCE=src/main
OBJ:=$(addsuffix .o, $(SOURCE))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)

all_optimization: o0 o2 o3

o0: $(OBJ)
	$(CC) $(OBJ) -o0 $(CFLAGS) -o thread_test_o0

o2: $(OBJ)
	$(CC) $(OBJ) -o2 $(CFLAGS) -o thread_test_o2

o3: $(OBJ)
	$(CC) $(OBJ) -o3 $(CFLAGS) -o thread_test_o3


src/%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf src/*.o $(TARGET) thread_test_o0 thread_test_o2 thread_test_o3

