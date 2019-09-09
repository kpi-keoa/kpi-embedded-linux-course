TARGET = multi 
DEPS = multi

DEPSO:=$(addsuffix .o, $(DEPS))
DEPSC:=$(addsuffix .c, $(DEPS))
DEPSASM:=$(addsuffix .s, $(DEPS))
DEPSI:=$(addsuffix .i, $(DEPS))

CC = gcc
CFLAGS = -lpthread -Wall -Wextra -Wpedantic
ASMFLAGS = -save-temps -fverbose-asm 

source_dir:= src
DEPSC:= $(addprefix $(source_dir)/, $(DEPSC))

WAY = /usr/local/bin/test
.PHONY: all clean O0 O1 O2 O3 Os install uninstall asm


all: clean $(TARGET)

$(TARGET): $(DEPSC)
	${CC} ${CFLAGS} $(DEPSC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(DEPSO) $(DEPSI) $(DEPSASM)

O0: $(DEPSC) 
	${CC} ${CFLAGS} $(DEPSC) -O0 -o $(TARGET)

O1: $(DEPSC) 
	${CC} ${CFLAGS} $(DEPSC) -O1 -o $(TARGET)
O2: $(DEPSC) 
	${CC} ${CFLAGS} $(DEPSC) -O2 -o $(TARGET)

O3: $(DEPSC)
	${CC} ${CFLAGS} $(DEPSC) -O3 -o $(TARGET)

Os: $(DEPSC)
	${CC} ${CFLAGS} $(DEPSC) -Os -o $(TARGET)
install:
	cp hash $(WAY)
uninstall: 
	rm -f $(WAY)

asm:$(DEPSC)
	${CC} ${CFLAGS} $(ASMFLAGS) $(DEPSC) -o $(TARGET)
