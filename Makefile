FLAGS=-g -Wall -pg -O4

stdlib_malloc:
	gcc -o test $(FLAGS)  dmalloc/ptmalloc3/malloc.c test.c

firstfit:
	gcc -o test $(FLAGS) -DSTRATEGY=1 malloc.c test.c

bestfit:
	gcc -o test $(FLAGS) -DSTRATEGY=2 malloc.c test.c

worstfit:
	gcc -o test $(FLAGS) -DSTRATEGY=3 malloc.c test.c

quickfit:
	gcc -o test $(FLAGS) -DSTRATEGY=4 -DNRQUICKLISTS=2 malloc.c test.c

robban:
	cp malloc.c test_os/ 
	cd test_os && make 

clean:

	rm -f *.o test *.out