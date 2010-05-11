FLAGS=-g -Wall



firstfit:
	gcc -o test $(FLAGS) -DSTRATEGY=1 malloc.c test.c

bestfit:
	gcc -o test $(FLAGS) -DSTRATEGY=2 malloc.c test.c

worstfit:
	gcc -o test $(FLAGS) -DSTRATEGY=3 malloc.c test.c


clean:

	rm -f *.o test 