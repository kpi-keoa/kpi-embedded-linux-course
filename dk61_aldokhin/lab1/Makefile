all: ttt

ttt: main.o
     gcc main.o -o -O0 -lpthread ttt

main.o: main.c
     gcc -c main.c

clean:
     rm -rf *.o ttt
