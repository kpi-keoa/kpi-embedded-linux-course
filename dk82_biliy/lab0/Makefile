TARGET    = my_dice
UTILS_DIR = ./utils
BUILD_DIR = ./build

.PHONY: all clean build main.o utils.o

all: build
	
build: $(TARGET)

clean:
			rm -rf $(BUILD_DIR)
main.o: main.c
			mkdir -p $(BUILD_DIR)
			gcc -c -o $(BUILD_DIR)/main.o main.c -I $(UTILS_DIR)
utils.o: $(UTILS_DIR)/utils.c
			gcc -c -o $(BUILD_DIR)/utils.o $(UTILS_DIR)/utils.c -I $(UTILS_DIR)
$(TARGET): main.o utils.o
			gcc -o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/main.o $(BUILD_DIR)/utils.o -I $(UTILS_DIR)
