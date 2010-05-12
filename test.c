#include "malloc.h"
#include <stdio.h>
int main(int argc, char *argv[]){
    /* Från labpek */
    fprintf(stderr,"------Test från labpek\n");   
    void * p = malloc(0);
    free(p);
    free(NULL);
    /* detta ska ge tillbaka 0 == NULL */
    #include <sys/resource.h>
    struct rlimit r;
    getrlimit(RLIMIT_DATA, &r);
    fprintf(stderr,"r.rlim_max: %d\n",r.rlim_max);
    p = malloc(2 * r.rlim_max);
    /* och dessa ? */
    p = realloc(NULL, 17);
    p = realloc(p,0);
    p = realloc(NULL,0);
    
    /*
    fprintf(stderr,"\nmalloc(8*2048) (bytes = 2048 units) \n");
    p = malloc(8*1000);
    p = malloc(8*1000);
    p = malloc(8*1000);
    fprintf(stderr,"\nrealloc(p,8*8) (8 units)\n");
    p = (int *)realloc(p,8*10);
    
    fprintf(stderr,"\nmalloc 8*10\n");
    p = malloc(8*10);
    */
    
    return 0;
}
