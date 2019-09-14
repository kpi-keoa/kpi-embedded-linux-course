TARGET = src/potoc

CC = gcc
CFLAGS = -O0 -Wall -Wextra -Wpedantic  -lpthread -save-temps -fverbose-asm -std=gnu99
LDFLAGS = -lrt

.PHONY: all clean help

all:	$(TARGET)	
$(TARGET):
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET))  -o $@ $(LDFLAGS)
	./$(TARGET) printed

	
	$(CC) $(CFLAGS) -O2 $(TARGET).c -o $(TARGET)
	./$(TARGET) printed 1 2 -O2
	@-rm -f $(TARGET)
	@-rm -f *.o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

help:				## display this message
	@echo  make is utility, which determines automatically which pieces of a large program need to be recompiled, and issues the commands to recompile them. 
	@echo  	##
	@echo  Available options:	
	@echo  make all the top-level targets the makefile knows about; 
	@echo  make clean - delete all files that are normally created by running make;
	@echo  make help - show this message.
clean:
	@echo Cleaning
	-rm -f $(TARGET)
	-rm -f *.o $(TARGET)
	-rm -f *.s $(TARGET)
	-rm -f printed
	
