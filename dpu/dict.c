#include <stdio.h>
#include "dict.h"

static void _dictReset(dict *ht)
{
    ht->used = 0;
    memset(ht->table, 0, ht->size * sizeof(unsigned int));
}
/* -------------------------- private prototypes ---------------------------- */

static unsigned int _dictNextPower(unsigned int size);
int dictGetHashedKey(dict *ht, const char *key);
static void _dictSetHashKey(dict *ht, __mram_ptr dictEntry *entry, const char *key_);
static void _dictSetHashVal(dict *ht, __mram_ptr dictEntry *entry, NodePtr val_);
static unsigned int _dictStringCopyHTHashFunction(const char *key);
static int _dictStringCopyHTKeyCompare(const char *key1, const __mram_ptr mram_str *key2_);

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

/* Identity hash function for integer keys */
unsigned int dictIdentityHashFunction(unsigned int key)
{
    return key;
}

// /* Generic hash function (a popular one from Bernstein).
//  * I tested a few and this was the best. */
// unsigned int dictGenHashFunction(const char *buf, int len)
// {
//     unsigned int hash = 5381;

//     while (len--)
//         hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
//     return hash;
// }

/* ----------------------------- API implementation ------------------------- */
/* Initialize the hash table */
int dictInit(dict *ht, mram_allocator *alloc)
{
    ht->size = DICT_HT_INITIAL_SIZE;
    ht->sizemask = ht->size - 1;
    ht->used = 0;
    ht->allocator = alloc;
    ht->table = (__mram_ptr unsigned int *)mram_alloc(ht->allocator, sizeof(unsigned int) * ht->size);
    memset(ht->table, 0, ht->size * sizeof(unsigned int));
    return DICT_OK;
}

/* Add an element to the target hash table */
int dictAdd(dict *ht, __mram_ptr char *key_, unsigned int key_len, NodePtr val_)
{
    __mram_ptr dictEntry *entry;

    char key[KEY_BUF_SIZE];
    
    mram_str_copy_to(key, key_, key_len + 1);
    int bucket= dictGetHashedKey(ht, key);
    // printf("%s ", key);
    
    // // mram_str_copy_to(val, val_, val_len + 1);
    // /* Get the index of the new element, or -1 if
    //  * the element already exists. */
    // if ((index = dictGetHashedKey(ht, key, bucket)) == -1)
    //     return DICT_ERR;

    // /* Allocates the memory and stores key */
    entry = (__mram_ptr dictEntry *)mram_alloc(ht->allocator, sizeof(dictEntry));
    entry->next = (__mram_ptr dictEntry *)(ht->table[bucket]);
    ht->table[bucket] = (unsigned int)entry;

    // /* Set the hash entry fields. */
    _dictSetHashKey(ht, entry, key);
    _dictSetHashVal(ht, entry, val_);

    ht->used++;
    // printf("%d\n", ht->used);
    return DICT_OK;
}

/* Add an element, discarding the old if the key already exists */
int dictReplace(dict *ht, __mram_ptr char *key_, unsigned int key_len, NodePtr new_val_)
{
    __mram_ptr dictEntry *entry;

    /* Try to add the element. If the key
     * does not exists dictAdd will suceed. */
    // if (dictAdd(ht, key_, key_len, val_, val_len, bucket) == DICT_OK)
    //     return DICT_OK;
    // char val[VAL_BUF_SIZE];
    // mram_str_copy_to(val, val_, val_len + 1);
    /* It already exists, get the entry */
    entry = dictFind(ht, key_, key_len);
    if (!entry)
        return DICT_ERR;
    /* Free the old value and set the new one */
    // dictFreeEntryVal(ht, entry);
    dictSetHashVal(ht, entry, new_val_);
    return DICT_OK;
}

/* Search and remove an element */
static int dictGenericDelete(dict *ht, __mram_ptr char *key_, unsigned int key_len, int nofree)
{
    unsigned int h;
    __mram_ptr dictEntry *he, *prevHe;

    char key[KEY_BUF_SIZE];
    mram_str_copy_to(key, key_, key_len + 1);
    if (ht->size == 0)
        return DICT_ERR;
    h = dictGetHashedKey(ht, key);
    he = (__mram_ptr dictEntry *)(ht->table[h]);

    prevHe = NULL;
    while (he)
    {
        if (dictCompareHashKeys(key, he->key))
        {
            /* Unlink the element from the list */
            if (prevHe)
                prevHe->next = he->next;
            else
                ht->table[h] = (unsigned int)(he->next);
            if (!nofree)
            {
                dictFreeEntryKey(ht, he);
                // dictFreeEntryVal(ht, he);
            }
            dictFreeEntry(ht, he);
            ht->used--;
            return DICT_OK;
        }
        prevHe = he;
        he = he->next;
    }
    return DICT_ERR; /* not found */
}

int dictDelete(dict *ht, __mram_ptr char *key_, unsigned int key_len)
{
    return dictGenericDelete(ht, key_, key_len, 0);
}

/* Destroy an entire hash table */
int _dictClear(dict *ht)
{
    unsigned int i;

    /* Free all the elements */
    for (i = 0; i < ht->size && ht->used > 0; i++)
    {
        __mram_ptr dictEntry *he, *nextHe;

        if ((he = (__mram_ptr dictEntry *)(ht->table[i])) == (__mram_ptr dictEntry *)NULL)
            continue;
        while (he)
        {
            nextHe = he->next;
            dictFreeEntryKey(ht, he);
            // dictFreeEntryVal(ht, he);
            dictFreeEntry(ht, he);
            ht->used--;
            he = nextHe;
        }
    }
    /* Re-initialize the table */
    _dictReset(ht);
    return DICT_OK; /* never fails */
}

/* Clear & Release the hash table */
void dictRelease(dict *ht)
{
    _dictClear(ht);
    /* Free the table and the allocated cache structure */
    mram_free(ht->allocator, (__mram_ptr void *)(ht->table));
}

__mram_ptr dictEntry *dictFind(dict *ht, __mram_ptr char *key_, unsigned int key_len)
{
    __mram_ptr dictEntry *he;
    unsigned int h;
    char key[KEY_BUF_SIZE];
    mram_str_copy_to(key, key_, key_len + 1);
    if (ht->size == 0)
        return (__mram_ptr dictEntry *)NULL;
    h = dictGetHashedKey(ht, key);;
    he = (__mram_ptr dictEntry *)(ht->table[h]);
    while (he)
    {
        if (dictCompareHashKeys(key, he->key))
            return he;
        he = he->next;
    }
    return (__mram_ptr dictEntry *)NULL;
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
int dictGetHashedKey(dict *ht, const char *key)
{
    // unsigned int h;
    // __mram_ptr dictEntry *he;

    // /* Compute the key hash value */
    // h = bucket;
    // /* Search if this slot does not already contain the given key */
    // he = (__mram_ptr dictEntry *)(ht->table[h]);
    // while (he)
    // {
    //     if (dictCompareHashKeys(key, he->key))
    //         return -1;
    //     he = he->next;
    // }
    // return h;
    return atoi(key)>>NR_DPUS_BITS;
}

void dictEmpty(dict *ht)
{
    _dictClear(ht);
}

/* ----------------------- StringCopy Hash Table Type ------------------------*/

static unsigned int _dictStringCopyHTHashFunction(const char *key)
{
    return dictGenHashFunction(key, strlen(key));
}

static int _dictStringCopyHTKeyCompare(const char *key1, const __mram_ptr mram_str *key2_)
{
    char key2[KEY_BUF_SIZE];
    mram_str_copy_to(key2, key2_->buf, key2_->len + 1);
    return strcmp(key1, key2) == 0;
}

static void _dictSetHashKey(dict *ht, __mram_ptr dictEntry *entry, const char *key_)
{
    __mram_ptr mram_str *key = mram_str_new(ht->allocator, key_);
    entry->key = key;
}

static void _dictSetHashVal(dict *ht, __mram_ptr dictEntry *entry, NodePtr val_)
{
    entry->val = val_;
}
