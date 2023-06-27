#ifndef __DICT_H
#define __DICT_H

#define NR_DPUS 1024
#define NR_DPUS_BITS 10
// #define BATCH_SIZE 64 * 64
// #define BATCH_SIZE 64
// #define KEY_NUM (1 << 16)
#define KEY_NUM (1 << 16)//数字大比如1 << 16就会出问题
#define FIND_NUM (1 << 16)
// #define MAX_QUERY_PER_DPU (KEY_NUM>>(NR_DPUS_BITS-1))//设置的稍微大一些，因为并不是每个dpu的request都一样，比如一个batch为64K，64个dpu，要设置的大于1024才行
#define MAX_QUERY_PER_DPU (KEY_NUM>>(NR_DPUS_BITS-1))//设置的稍微大一些，因为并不是每个dpu的request都一样，比如一个batch为64K，64个dpu，要设置的大于1024才行
#define BATCH_SIZE (MAX_QUERY_PER_DPU)


#define DICT_OK 0
#define DICT_ERR 1

#define KEY_BUF_SIZE 8
#define VAL_BUF_SIZE 32
/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     65536
#include "mram_alloc.h"
#include "mram_str.h"
typedef struct Node 
{
    char val[8];
} Node;
typedef unsigned long long NodePtr;

typedef struct dictEntry {
    __mram_ptr mram_str *key;
    NodePtr val;
    __mram_ptr struct dictEntry *next;
} dictEntry;

typedef struct dict {
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;
    mram_allocator *allocator;
    __mram_ptr unsigned int *table;
} dict;

/* ------------------------------- Macros ------------------------------------*/
// #define dictFreeEntryVal(ht, entry) mram_free(ht->allocator,entry->val)
#define dictSetHashVal(ht, entry, _val_) _dictSetHashVal(ht, entry, _val_)
#define dictFreeEntryKey(ht, entry) mram_free(ht->allocator,entry->key)
#define dictSetHashKey(ht, entry, _key_) _dictSetHashKey(ht, entry, _key_)
#define dictFreeEntry(ht, entry) mram_free(ht->allocator,entry)

#define dictCompareHashKeys(key1, key2) _dictStringCopyHTKeyCompare(key1,key2)
#define dictHashKey(key) _dictStringCopyHTHashFunction(key)

#define dictGetEntryKey(he) ((he)->key)
#define dictGetEntryVal(he) ((he)->val)
#define dictSlots(ht) ((ht)->size)
#define dictSize(ht) ((ht)->used)

/* API */
int dictInit(dict *ht,mram_allocator *alloc);
int dictAdd(dict *ht, __mram_ptr char *key_,unsigned int key_len, NodePtr val_);
int dictReplace(dict *ht, __mram_ptr char *key_,unsigned int key_len, NodePtr val_);
int dictDelete(dict *ht,__mram_ptr char *key_,unsigned int key_len);
void dictRelease(dict *ht);
__mram_ptr dictEntry * dictFind(dict *ht,__mram_ptr char *key_,unsigned int key_len);
unsigned int dictGenHashFunction(const char *buf, int len);
void dictEmpty(dict *ht);

#endif /* __DICT_H */
