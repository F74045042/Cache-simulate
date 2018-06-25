all: simulate.o
	gcc -o simulate simulate.o

simulate.o: simulate.c
	gcc -c simulate.c
	
clean: 
	rm simulate *.o *.txt