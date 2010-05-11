#include "malloc.h"
#include <stdio.h>
int main(int argc, char *argv[]){
  fprintf(stderr,"\nmalloc(8*2048) (bytes = 2048 units) \n");
  int * p = malloc(8*2048);
  fprintf(stderr,"\nrealloc(p,8*8) (8 units)\n");
  p = (int *)realloc(p,8*8);
  fprintf(stderr,"\nmalloc 8*10\n");
  p = malloc(8*10);
  
  return 0;
}
