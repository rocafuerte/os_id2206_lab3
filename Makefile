FLAGS=-g -Wall


all: 
	gcc -o test $(FLAGS) malloc.c test.c

clean:
	rm -f *.o test 