#ifndef __MRAM_STR_H
#define __MRAM_STR_H
#include "mram_alloc.h"
#include <stdlib.h>
#include <string.h>
#define MRAM_STR_MAXLEN 32
typedef struct mram_str
{
    unsigned int len;
    char buf[];
} mram_str;

__mram_ptr mram_str *mram_str_new(mram_allocator *allocator,const char *init);
void mram_str_copy_from(__mram_ptr char *dst,const char *src,size_t size);
void mram_str_copy_to(char *dst,const __mram_ptr char *src,size_t size);
void mram_str_print(__mram_ptr mram_str *str);
#endif
