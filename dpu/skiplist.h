#ifndef _ZSKIPLIST_H
#define _ZSKIPLIST_H
#define ZSKIPLIST_MAXLEVEL 16
#define ZSKIPLIST_P 0.25
#include "mram_alloc.h"

typedef struct {
    double min, max;
    int minex, maxex; /* are min or max exclusive? */
} zrangespec;
typedef struct zskiplistNode {
    int ele;
    double score;
    __mram_ptr struct zskiplistNode *backward;
    struct zskiplistLevel {
        __mram_ptr struct zskiplistNode *forward;
        unsigned int span;
    } level[];
} zskiplistNode;

typedef struct zskiplist {
    __mram_ptr struct zskiplistNode *header, *tail;
    unsigned long length;
    int level;
    mram_allocator *allocator;
} zskiplist;

void zslCreate(zskiplist *zsl,mram_allocator *alloc);
void zslFree(zskiplist *zsl);
__mram_ptr zskiplistNode *zslInsert(zskiplist *zsl, double score, int ele);
int zslDelete(zskiplist *zsl, double score, int ele);
__mram_ptr zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range);
__mram_ptr zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range);
__mram_ptr zskiplistNode *zslFind(zskiplist *zsl, double score, int ele);
#endif