#ifndef _MALLOC_H
#define _MALLOC_H 1
#include <stdlib.h>
#include <unistd.h>

void *malloc(size_t size);
void *realloc(void *ptr, size_t  size);
void free(void *ptr);

#endif
