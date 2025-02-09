#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1
/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE_PER_PDU 512
#define DICT_HT_INITIAL_SIZE (DICT_HT_INITIAL_SIZE_PER_PDU * NR_DPUS)
#include "connect.h"
#include <stdlib.h>
typedef struct dict
{
    unsigned int size;
    unsigned int sizemask;
    unsigned int used;
} dict;

/* API */
dict *dictInit();
int dictGetHashedKey(dict *ht, const char *key_);
int dictAdd(dict *ht, request_batch *rqst, const char *key_, NodePtr val_);
int dictAddSpecificDPU(dict *ht, request_batch *rqst, int dpu, const char *key_, NodePtr val_, int max_size_per_dpu);
int dictReplace(dict *ht, request_batch *rqst, const char *key_, NodePtr val_);
int dictDelete(dict *ht, request_batch *rqst, const char *key_);
int dictFind(dict *ht, request_batch *rqst, const char *key_);
int dictFindSpecificDPU(dict *ht, request_batch *rqst, int dpu, const char *key_, int max_size_per_dpu);
unsigned int dictGenHashFunction(const char *buf, int len);

#endif /* __DICT_H */
