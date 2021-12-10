CC = gcc

MAIN = main
RAND = rand

$(MAIN): $(MAIN).o $(RAND).o
	$(CC) -std=gnu11 -Wall -o $@ $^
	
$(MAIN).o: $(MAIN).c
	$(CC) -c $^ -std=gnu11 -Wall
	
$(RAND).o: $(RAND).c $(RAND).h
	$(CC) -c $^ -std=gnu11 -Wall
	
clean:
	rm -f *.o
	rm -f *.h.gch
