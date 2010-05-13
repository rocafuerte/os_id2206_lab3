#ifndef _malloc_h_
#define _malloc_h_
#include <stdio.h> /* Behöver man denna */

extern void *malloc(size_t);
extern void *realloc(void *, size_t);
extern void free(void *);

#endif
