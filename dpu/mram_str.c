#include "mram_str.h"
#include <string.h>
#include <stdio.h>

void mram_str_copy_from(__mram_ptr char *dst, const char *src, size_t size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
}
void mram_str_copy_to(char *dst, const __mram_ptr char *src, size_t size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
}
__mram_ptr mram_str *mram_str_new(mram_allocator *allocator, const char *init)
{
    unsigned int len = strlen(init);
    __mram_ptr mram_str *str = mram_alloc(allocator, sizeof(mram_str) + len + 1);
    str->len = len;
    mram_str_copy_from(str->buf, init, len + 1);
    return str;
}

void mram_str_print(__mram_ptr mram_str *str)
{
    char tmp[MRAM_STR_MAXLEN];
    mram_str_copy_to(tmp, str->buf, str->len + 1);
    printf("mram_str:%s,strlen:%d\n", tmp, strlen(tmp));
}