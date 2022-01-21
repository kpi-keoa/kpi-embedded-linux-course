CC = gcc

MAIN = lab0


$(MAIN): $(MAIN).o 
	$(CC) -std=gnu11 -Wall -o $@ $^
	
$(MAIN).o: $(MAIN).c
	$(CC) -c $^ -std=gnu11 -Wall

	
clean:
	rm -f *.o
	rm -f *.h.gch

