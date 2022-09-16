#include"skiplist.h"
#include<stdlib.h>

/* Create a skiplist node with the specified number of levels.
 * The sds string 'ele' is referenced by the node after the call. */
static unsigned long int next = 1;
int rand(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}
__mram_ptr zskiplistNode *zslCreateNode(int level, double score, int ele, mram_allocator *alloc) {
    __mram_ptr zskiplistNode *zn =
        (__mram_ptr zskiplistNode *)mram_alloc(alloc,sizeof(*zn)+level*sizeof(struct zskiplistLevel));
    zn->score = score;
    zn->ele = ele;
    return zn;
}

/* Create a new skiplist. */
void zslCreate(zskiplist *zsl,mram_allocator *alloc) {
    int j;

    zsl->allocator=alloc;
    zsl->level = 1;
    zsl->length = 0;
    zsl->header = zslCreateNode(ZSKIPLIST_MAXLEVEL,0,0,zsl->allocator);
    for (j = 0; j < ZSKIPLIST_MAXLEVEL; j++) {
        zsl->header->level[j].forward = (__mram_ptr zskiplistNode *)NULL;
        zsl->header->level[j].span = 0;
    }
    zsl->header->backward = (__mram_ptr zskiplistNode *)NULL;
    zsl->tail = (__mram_ptr zskiplistNode *)NULL;
    return;
}
void zslFreeNode(__mram_ptr zskiplistNode *node) {
    // free(node);
}

/* Free a whole skiplist. */
void zslFree(zskiplist *zsl) {
    __mram_ptr zskiplistNode *node = zsl->header->level[0].forward, *next;

    zslFreeNode(zsl->header);
    while(node) {
        next = node->level[0].forward;
        zslFreeNode(node);
        node = next;
    }
}

int zslRandomLevel(void) {
    int level = 1;
    while ((rand()&0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
        level += 1;
    return (level<ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

/* Insert a new node in the skiplist. Assumes the element does not already
 * exist (up to the caller to enforce that). The skiplist takes ownership
 * of the passed sds string 'ele'. */
__mram_ptr zskiplistNode *zslInsert(zskiplist *zsl, double score, int ele) {
    __mram_ptr zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    unsigned int rank[ZSKIPLIST_MAXLEVEL];
    int i, level;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        /* store rank that is crossed to reach the insert position */
        rank[i] = i == (zsl->level-1) ? 0 : rank[i+1];
        while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                    (x->level[i].forward->score == score &&
                    x->level[i].forward->ele < ele)))
        {
            rank[i] += x->level[i].span;
            x = x->level[i].forward;
        }
        update[i] = x;
        // printf("%p,%d\n",update[i],update[i]->ele);
    }
    /* we assume the element is not already inside, since we allow duplicated
     * scores, reinserting the same element should never happen since the
     * caller of zslInsert() should test in the hash table if the element is
     * already inside or not. */
    level = zslRandomLevel();
    if (level > zsl->level) {
        for (i = zsl->level; i < level; i++) {
            rank[i] = 0;
            update[i] = zsl->header;
            update[i]->level[i].span = zsl->length;
        }
        zsl->level = level;
    }
    x = zslCreateNode(level,score,ele,zsl->allocator);
    for (i = 0; i < level; i++) {
        x->level[i].forward = update[i]->level[i].forward;
        update[i]->level[i].forward = x;

        /* update span covered by update[i] as x is inserted here */
        x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
        update[i]->level[i].span = (rank[0] - rank[i]) + 1;
    }

    /* increment span for untouched levels */
    for (i = level; i < zsl->level; i++) {
        update[i]->level[i].span++;
    }

    x->backward = (update[0] == zsl->header) ? NULL : update[0];
    if (x->level[0].forward)
        x->level[0].forward->backward = x;
    else
        zsl->tail = x;
    zsl->length++;
    return x;
}

/* Internal function used by zslDelete, zslDeleteByScore and zslDeleteByRank */
void zslDeleteNode(zskiplist *zsl, __mram_ptr zskiplistNode *x, __mram_ptr zskiplistNode **update) {
    int i;
    // __mram_ptr zskiplistNode *update[ZSKIPLIST_MAXLEVEL];
    for (i = 0; i < zsl->level; i++) {
        // update[i]=(__mram_ptr zskiplistNode *)(update_ptr[i]);
        if (update[i]->level[i].forward == x) {
            update[i]->level[i].span += x->level[i].span - 1;
            update[i]->level[i].forward = x->level[i].forward;
        } else {
            update[i]->level[i].span -= 1;
        }
    }
    if (x->level[0].forward) {
        x->level[0].forward->backward = x->backward;
    } else {
        zsl->tail = x->backward;
    }
    while(zsl->level > 1 && zsl->header->level[zsl->level-1].forward == NULL)
        zsl->level--;
    zsl->length--;
}

int zslDelete(zskiplist *zsl, double score, int ele) {
    __mram_ptr zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    int i;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                    (x->level[i].forward->score == score &&
                     x->level[i].forward->ele < ele)))
        {
            x = x->level[i].forward;
        }
        update[i] = x;
    }
    /* We may have multiple elements with the same score, what we need
     * is to find the element with both the right score and object. */
    x = x->level[0].forward;
    if (x && score == x->score && x->ele == ele) {
        // unsigned int update_ptr[ZSKIPLIST_MAXLEVEL];
        // for(i=0;i<zsl->level;i++){
        //     update_ptr[i]=(unsigned int)(update[i]);
        // }
        zslDeleteNode(zsl, x, update);
        return 1;
    }
    return 0; /* not found */
}

int zslValueGteMin(double value, zrangespec *spec) {
    return spec->minex ? (value > spec->min) : (value >= spec->min);
}

int zslValueLteMax(double value, zrangespec *spec) {
    return spec->maxex ? (value < spec->max) : (value <= spec->max);
}

/* Returns if there is a part of the zset is in range. */
int zslIsInRange(zskiplist *zsl, zrangespec *range) {
    __mram_ptr zskiplistNode *x;

    /* Test for ranges that will always be empty. */
    if (range->min > range->max ||
            (range->min == range->max && (range->minex || range->maxex)))
        return 0;
    x = zsl->tail;
    if (x == NULL || !zslValueGteMin(x->score,range))
        return 0;
    x = zsl->header->level[0].forward;
    if (x == NULL || !zslValueLteMax(x->score,range))
        return 0;
    return 1;
}
__mram_ptr zskiplistNode *zslFind(zskiplist *zsl, double score, int ele) {
    __mram_ptr zskiplistNode *x;
    int i;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                    (x->level[i].forward->score == score &&
                     x->level[i].forward->ele < ele)))
        {
            x = x->level[i].forward;
        }
    }
    /* We may have multiple elements with the same score, what we need
     * is to find the element with both the right score and object. */
    x = x->level[0].forward;
    if (x && score == x->score && x->ele == ele) {
        return x;
    }
    return (__mram_ptr zskiplistNode *)NULL; /* not found */
}
/* Find the first node that is contained in the specified range.
 * Returns NULL when no element is contained in the range. */
__mram_ptr zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range) {
    __mram_ptr zskiplistNode *x;
    int i;

    /* If everything is out of range, return early. */
    if (!zslIsInRange(zsl,range)) return (__mram_ptr zskiplistNode *)NULL;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        /* Go forward while *OUT* of range. */
        while (x->level[i].forward &&
            !zslValueGteMin(x->level[i].forward->score,range))
                x = x->level[i].forward;
    }

    /* This is an inner range, so the next node cannot be NULL. */
    x = x->level[0].forward;

    /* Check if score <= max. */
    if (!zslValueLteMax(x->score,range)) return (__mram_ptr zskiplistNode *)NULL;
    return x;
}

/* Find the last node that is contained in the specified range.
 * Returns NULL when no element is contained in the range. */
__mram_ptr zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range) {
    __mram_ptr zskiplistNode *x;
    int i;

    /* If everything is out of range, return early. */
    if (!zslIsInRange(zsl,range)) return (__mram_ptr zskiplistNode *)NULL;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        /* Go forward while *IN* range. */
        while (x->level[i].forward &&
            zslValueLteMax(x->level[i].forward->score,range))
                x = x->level[i].forward;
    }

    /* This is an inner range, so this node cannot be NULL. */

    /* Check if score >= min. */
    if (!zslValueGteMin(x->score,range)) return (__mram_ptr zskiplistNode *)NULL;
    return x;
}