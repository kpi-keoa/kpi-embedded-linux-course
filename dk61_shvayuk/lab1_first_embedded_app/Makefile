.PHONY : clean all

all: clean
	mkdir build
	gcc src/example_mt.c -O0 -o build/example_mt_O0 -Wall -Wpedantic -pthread
	gcc src/example_mt.c -O2 -o build/example_mt_O2 -Wall -Wpedantic -pthread
clean:
	clear; clear
	rm -rfv build
	
