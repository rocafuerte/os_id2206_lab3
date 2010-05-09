#include "malloc.h"
int main(int argc, char *argv[]){
  
  int * a  = (int *) malloc(sizeof(int));
  *a = 2;
  return 0;
}
