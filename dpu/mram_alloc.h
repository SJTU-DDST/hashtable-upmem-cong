#ifndef _MRAM_ALLOC_H
#define _MRAM_ALLOC_H
#include <stddef.h>
#include <mram.h>

#define MRAM_HEAP_START_PTR ((__mram_ptr void *)DPU_MRAM_HEAP_POINTER)
#define MRAM_HEAP_END_PTR ((__mram_ptr void *)(0x4000000))

typedef struct mram_allocator
{
    __mram_ptr void *start;
    __mram_ptr void *end;
    __mram_ptr void *cur;
    size_t avail_size;
    size_t mram_size;
} mram_allocator;

void mram_allocator_initial(mram_allocator* allocator);
__mram_ptr void * mram_alloc(mram_allocator* allocator,size_t size);
void mram_free(mram_allocator* allocator,__mram_ptr void *ptr);
#endif