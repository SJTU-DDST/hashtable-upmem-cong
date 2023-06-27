#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"

static unsigned int _dictNextPower(unsigned int size);
int dictGetHashedKey(dict *ht, const char *key);

/* -------------------------- hash functions -------------------------------- */

// /* Thomas Wang's 32 bit Mix Function */
// unsigned int dictIntHashFunction(unsigned int key)
// {
//     key += ~(key << 15);
//     key ^= (key >> 10);
//     key += (key << 3);
//     key ^= (key >> 6);
//     key += ~(key << 11);
//     key ^= (key >> 16);
//     return key;
// }

/* Generic hash function (a popular one from Bernstein).
 * I tested a few and this was the best. */
// unsigned int dictGenHashFunction(const char *buf, int len)
// {
//     // unsigned int hash = 5381;

//     // while (len--)
//     //     hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
//     // return hash;
//     return atoi(buf)%NR_DPUS;
// }


/* ----------------------------- API implementation ------------------------- */
/* Initialize the hash table */
dict *dictInit()
{
    dict *ht = malloc(sizeof(dict));
    ht->size = DICT_HT_INITIAL_SIZE;
    ht->sizemask = ht->size - 1;
    ht->used = 0;
    return ht;
}


// int dictGetHashedKey(dict *ht, const char *key)
// {
//     return dictGetHashedKey(ht, key);
// }

/* Add an element to the target hash table */
int dictAdd(dict *ht, request_batch *rqst, const char *key_, NodePtr node_)
{
    int index;

    index = dictGetHashedKey(ht, key_);

    requestAdd(rqst, index, INSERT, key_, node_);
    return DICT_OK;
}

int dictAddSpecificDPU(dict *ht, request_batch *rqst, int dpu, const char *key_, NodePtr node_, int max_size_per_dpu)
{
    int index;

    // index = dictGetHashedKey(ht, key_);
    // printf("index %d, dpu %d\n", index, dpu);

    // requestAddSpecificDPU(rqst, dictGetHashedKey(ht, key_) % NR_DPUS, INSERT, index, key_, node_, max_size_per_dpu);
    requestAddSpecificDPU(rqst, dpu, INSERT, key_, node_, max_size_per_dpu);
    
    // if(dpu!=dictGetHashedKey(ht, key_) % NR_DPUS)
    // {
    //     printf("dpu: %d\n", dpu);
    //     printf("dictGetHashedKey(ht, key_)  NR_DPUS2: %d\n", dictGetHashedKey(ht, key_) % NR_DPUS);
    // }
    return DICT_OK;
}

/* Add an element, discarding the old if the key already exists */
int dictReplace(dict *ht, request_batch *rqst, const char *key_, NodePtr val_)
{
    int index;

    index = dictGetHashedKey(ht, key_);

    requestAdd(rqst, index, REPLACE, key_, val_);
    return DICT_OK;
}

int dictDelete(dict *ht, request_batch *rqst, const char *key_)
{
    int index;

    index = dictGetHashedKey(ht, key_);

    requestAdd(rqst, index, DELETE, key_, NULL);
    return DICT_OK;
}

int dictFind(dict *ht, request_batch *rqst, const char *key_)
{
    int index;

    index = dictGetHashedKey(ht, key_);

    requestAdd(rqst, index, FIND, key_, NULL);
    return DICT_OK;
}

int dictFindSpecificDPU(dict *ht, request_batch *rqst, int dpu, const char *key_, int max_size_per_dpu)
{
    int index;

    // index = dictGetHashedKey(ht, key_);

    // printf("find index %d, dpu %d\n", index, dpu);

    requestAddSpecificDPU(rqst, dpu, FIND, key_, NULL, max_size_per_dpu);

    return DICT_OK;
}

/* ------------------------- private functions ------------------------------ */
/* Our hash table capability is a power of two */
static unsigned int _dictNextPower(unsigned int size)
{
    unsigned int i = DICT_HT_INITIAL_SIZE;

    if (size >= 2147483648U)
        return 2147483648U;
    while (1)
    {
        if (i >= size)
            return i;
        i *= 2;
    }
}

/* Returns the index of a free slot that can be populated with
 * an hash entry for the given 'key'.
 * If the key already exists, -1 is returned. */
//已经模过NR_DPUS了
int dictGetHashedKey(dict *ht, const char *key)
{
    unsigned int h;

    /* Compute the key hash value */
    // h = dictGenHashFunction(key, strlen(key)) & ht->sizemask;
    // return h;
    return atoi(key)%NR_DPUS;
}
