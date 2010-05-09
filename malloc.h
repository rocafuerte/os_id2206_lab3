#ifndef _MALLOC_H
#define _MALLOC_H 1
#include <stdlib.h>
#include <unistd.h>

#define NALLOC 1024
typedef long Align; /* Ändra till rätt */


union header{
  struct{
    union header *ptr;
    size_t size; /* unisgned vad? */
  } s;
  Align x;
};

typedef union header Header;

void *malloc(size_t size);
void *realloc(void *ptr, size_t  size);
void free(void *ptr);
static Header *morecore(size_t nu);
#endif
