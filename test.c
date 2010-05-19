#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#define NITS 50000
#define MSIZE 8
#define TIMES 1000
#define RANDINT 512

int i;
void test1_small(){
    char* p[NITS];
    for(i=0;i<NITS;i++){
        p[i]=malloc(MSIZE);
        if(p[i]==NULL){
            fprintf(stderr,"it %d: malloc returned NULL\n",i);
            return;
        }
    }
    for(i=0;i<NITS;i++){
        free(p[i]);
    }
}

void test2_random(){
    char* p[NITS];
    for(i=0;i<NITS;i++){
        p[i]=malloc((rand()%RANDINT)+1);
        if(p[i]==NULL){
            fprintf(stderr,"it %d: malloc returned NULL\n",i);
            return;
        }
    }            
    for(i=0;i<NITS;i++){
        free(p[i]);
    }    
}

void test3_random(){
    char* p[NITS];
    for(i=1;i<NITS;i++){
        p[i]=malloc((rand()%RANDINT)+1);
        if(p[i]==NULL){
            fprintf(stderr,"it %d: malloc returned NULL\n",i);
            return;
        }
        if(rand()%2==0) {
            int t= rand()%i;
            p[t]=realloc(p[t],(rand()%RANDINT)+1);
        }  
    }
    for(i=1;i<NITS;i++){
        free(p[i]);
    }    
}

void test(){
    char *p = malloc(1024);
    print_free_lists();
    realloc(p,1023);
    print_free_lists();
    /*malloc(8*128);
      print_free_lists();*/
}

int main(int argc, char *argv[]){    
    /*srand(time(NULL));*/
    /* test1_small(); */
    /*test2_random();*/
    /*test3_random();*/
    test(); 
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
    /*
    print_free_lists();
    char * p = malloc(8*3);
    print_free_lists();
    p = malloc(8*3);
    print_free_lists();
    malloc(8*1);
    */
    /*realloc(p,0);
      realloc(p,8*2);*/

    return 0;
}
