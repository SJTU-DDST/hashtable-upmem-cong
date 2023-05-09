#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1
/* This is the initial size of every hash table */

#define DICT_HT_INITIAL_SIZE 65536
#include "connect.h"
typedef struct dict
{
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;
    unsigned int *table;
} dict;

typedef struct dictEntry
{
    char key[8];
    NodePtr val;
    struct dictEntry *next;
} dictEntry;

/* API */
dict *dictInit();
int dictGetHashedKey(dict *ht, const char *key_);
int dictAdd(dict *ht, request_batch *rqst, const char *key_, NodePtr val_);
// int dictReplace(dict *ht, request_batch *rqst, const char *key_, NodePtr val_);
// int dictDelete(dict *ht, request_batch *rqst, const char *key_);
int dictFind(dict *ht, request_batch *rqst, const char *key_);
unsigned int dictGenHashFunction(const char *buf, int len);
int dictAddCPU(dict *ht, char *key_, unsigned int key_len, NodePtr val_, unsigned int bucket);
dictEntry* dictFindCPU(dict *ht, char *key_, unsigned int key_len, unsigned int bucket);

#endif /* __DICT_H */
