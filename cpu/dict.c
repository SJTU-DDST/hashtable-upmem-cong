#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"

static unsigned int _dictNextPower(unsigned int size);
static void _dictSetHashKey(dict *ht, dictEntry *entry, const char *key_);
static void _dictSetHashVal(dict *ht, dictEntry *entry, NodePtr val_);
static int _dictKeyIndex(dict *ht, const char *key);
static int _dictKeyIndexCPU(dict *ht, const char *key, unsigned int bucket);

/* -------------------------- hash functions -------------------------------- */

/* Thomas Wang's 32 bit Mix Function */
unsigned int dictIntHashFunction(unsigned int key)
{
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

/* Generic hash function (a popular one from Bernstein).
 * I tested a few and this was the best. */
unsigned int dictGenHashFunction(const char *buf, int len)
{
    unsigned int hash = 5381;

    while (len--)
        hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
    return hash;
}

/* ----------------------------- API implementation ------------------------- */
/* Initialize the hash table */
dict *dictInit()
{
    dict *ht = malloc(sizeof(dict));
    ht->size = DICT_HT_INITIAL_SIZE;
    ht->sizemask = ht->size - 1;
    ht->used = 0;
    ht->table = (unsigned int *)malloc(sizeof(unsigned int) * ht->size);
    memset(ht->table, 0, ht->size * sizeof(unsigned int));
    return ht;
}

int dictGetHashedKey(dict *ht, const char *key)
{
    return _dictKeyIndex(ht, key);
}

/* Add an element to the target hash table */
int dictAdd(dict *ht, request_batch *rqst, const char *key_, NodePtr node_)
{
    int index;

    index = _dictKeyIndex(ht, key_);

    // printf("%d\n", index);

    requestAdd(rqst, INSERT, index, key_, node_);
    return DICT_OK;
}

int dictAddCPU(dict *ht, char *key_, unsigned int key_len, NodePtr val_, unsigned int bucket)
{
    // printf("%s %#llx %d\n", key_, val_, bucket);
    int index;
    dictEntry *entry;

    char key[KEY_BUF_SIZE];
    
    strncpy(key, key_, key_len + 1);

    
    // // mram_str_copy_to(val, val_, val_len + 1);
    // /* Get the index of the new element, or -1 if
    //  * the element already exists. */
    if ((index = _dictKeyIndexCPU(ht, key, bucket)) == -1)
        return DICT_ERR;

    // /* Allocates the memory and stores key */
    entry = (dictEntry *)malloc(sizeof(dictEntry));
    entry->next = (dictEntry *)(ht->table[index]);
    ht->table[index] = (unsigned int)entry;

    // /* Set the hash entry fields. */
    _dictSetHashKey(ht, entry, key);
    _dictSetHashVal(ht, entry, val_);

    ht->used++;
    return DICT_OK;
}

// /* Add an element, discarding the old if the key already exists */
// int dictReplace(dict *ht, request_batch *rqst, const char *key_, NodePtr val_)
// {
//     int index;

//     index = _dictKeyIndex(ht, key_);

//     requestAdd(rqst, index / DICT_HT_INITIAL_SIZE_PER_PDU, REPLACE, index % DICT_HT_INITIAL_SIZE_PER_PDU, key_, val_);
//     return DICT_OK;
// }

// int dictDelete(dict *ht, request_batch *rqst, const char *key_)
// {
//     int index;

//     index = _dictKeyIndex(ht, key_);

//     requestAdd(rqst, index / DICT_HT_INITIAL_SIZE_PER_PDU, DELETE, index % DICT_HT_INITIAL_SIZE_PER_PDU, key_, NULL);
//     return DICT_OK;
// }

int dictFind(dict *ht, request_batch *rqst, const char *key_)
{
    int index;

    index = _dictKeyIndex(ht, key_);

    requestAdd(rqst, FIND, index, key_, NULL);
    return DICT_OK;
}

dictEntry* dictFindCPU(dict *ht, char *key_, unsigned int key_len, unsigned int bucket)
{
    // printf("dictfind: %s %d\n", key_, bucket);
    dictEntry *he;
    unsigned int h;
    char key[KEY_BUF_SIZE];
    strncpy(key, key_, key_len + 1);
    if (ht->size == 0)
        return (dictEntry *)NULL;
    h = bucket;
    he = (dictEntry *)(ht->table[h]);
    while (he)
    {
        if (strcmp(key, he->key) == 0)
            return he;
        he = he->next;
    }
    return (dictEntry *)NULL;
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
static int _dictKeyIndex(dict *ht, const char *key)
{
    unsigned int h;

    /* Compute the key hash value */
    h = dictGenHashFunction(key, strlen(key)) & ht->sizemask;

    return h;
}

/* Returns the index of a free slot that can be populated with
 * an hash entry for the given 'key'.
 * If the key already exists, -1 is returned. */
static int _dictKeyIndexCPU(dict *ht, const char *key, unsigned int bucket)
{
    unsigned int h;
    dictEntry *he;

    /* Compute the key hash value */
    h = bucket;

    /* Search if this slot does not already contain the given key */
    he = (dictEntry *)(ht->table[h]);
    while (he)
    {
        if (strcmp(key, he->key) == 0)
            return -1;
        he = he->next;
    }
    return h;
}


/* ----------------------- StringCopy Hash Table Type ------------------------*/

// static unsigned int _dictStringCopyHTHashFunction(const char *key)
// {
//     return dictGenHashFunction(key, strlen(key));
// }

// static int _dictStringCopyHTKeyCompare(const char *key1, const __mram_ptr mram_str *key2_)
// {
//     char key2[KEY_BUF_SIZE];
//     mram_str_copy_to(key2, key2_->buf, key2_->len + 1);
//     return strcmp(key1, key2) == 0;
// }

static void _dictSetHashKey(dict *ht, dictEntry *entry, const char *key_)
{
    strcpy(entry->key, key_);
}

static void _dictSetHashVal(dict *ht, dictEntry *entry, NodePtr val_)
{
    entry->val = val_;
}

