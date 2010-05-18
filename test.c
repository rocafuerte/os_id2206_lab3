#include "malloc.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#define NITS 1000000
#define MSIZE 8
#define TIMES 1000
int main(int argc, char *argv[]){    
    /*
    int msize = 0;
    char * addr[NITS];
    int i;
    for(i=0;i<NITS; ++i){
        msize = (i%MSIZE)+1;
        addr[i] = malloc(msize);
    }
    */

/*     srand(time(NULL)); */
/*     int i; */
/*     double * p = malloc(245988); */
/*     for(i=0;i<100000;i++){        */
/*         /\*p = (double*)malloc((rand()%1024)*sizeof(double));        *\/ */
/*         p = (double*)malloc((128+i)*sizeof(double)); */
/*     } */
    print_free_lists();
    char * p = malloc(8*3);
    print_free_lists();
    p = malloc(8*4);
    /*realloc(p,8*1);*/
    /*realloc(p,0);
      realloc(p,8*2);*/
    
    print_free_lists();
    return 0;
}
