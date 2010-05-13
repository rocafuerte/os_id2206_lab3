#include "malloc.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#define NITS 1000000
#define MSIZE 8
int r;
int main(int argc, char *argv[]){    
    int msize = 0;
    char * addr[NITS];
    int i;
    for(i=0;i<NITS; ++i){        
        msize = (i%MSIZE)+1;
        addr[i] = malloc(msize);
    }
    return 0;
}
