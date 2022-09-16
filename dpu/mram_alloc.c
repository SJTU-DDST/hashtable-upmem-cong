#include "mram_alloc.h"

void mram_allocator_initial(mram_allocator *allocator)
{
    allocator->start = MRAM_HEAP_START_PTR;
    allocator->end = MRAM_HEAP_END_PTR;
    allocator->cur = MRAM_HEAP_START_PTR;
    allocator->mram_size = (size_t)MRAM_HEAP_END_PTR - (size_t)MRAM_HEAP_START_PTR;
    allocator->avail_size = allocator->mram_size;
}

__mram_ptr void *mram_alloc(mram_allocator *allocator, size_t size)
{
    if (size % 8)
    {
        size = (size / 8 + 1) * 8;
    }
    if (allocator->avail_size < size)
        return (__mram_ptr void *)NULL;
    __mram_ptr void *ptr = allocator->cur;
    allocator->cur = (__mram_ptr void *)(allocator->cur + size);
    allocator->avail_size = allocator->avail_size - size;
    return ptr;
}
void mram_free(mram_allocator *allocator, __mram_ptr void *ptr)
{
    return;
}