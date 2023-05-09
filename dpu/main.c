#include <mram.h>
#include <alloc.h>
#include <stdint.h>
#include <stdio.h>
#include "mram_alloc.h"
#include "mram_str.h"
#include "dict.h"

#define BATCH_SIZE 64 * 64
#define KEY_BUF_SIZE 8
#define VAL_BUF_SIZE 32

// operate
#define INSERT 0
#define FIND 1
#define DELETE 2
#define REPLACE 3

// reponse
#define DICT_ADD_OK 0
#define DICT_ADD_ERR 1
#define DICT_REPLACE_OK 2
#define DICT_REPLACE_ERR 3
#define DICT_DELETE_OK 4
#define DICT_DELETE_ERR 5
#define DICT_FIND_OK 6
#define DICT_FIND_ERR 7

typedef struct key__
{
    unsigned int len;
    char buf[KEY_BUF_SIZE];
} key__;
typedef struct value__
{
    NodePtr val;
} value__;
typedef struct request
{
    unsigned int operate; // 0:insert,1:find,2:delete,3:replace
    unsigned int bucket;
    key__ key;
    value__ val;
} request;
typedef struct response
{
    unsigned int response; // 0:OK,1:ERROR
    value__ val;
} response;

/* Buffer in MRAM. */
__host unsigned int rqst_size = 0;
__mram_noinit unsigned int size[2];
__mram_noinit request rqst[BATCH_SIZE];
__mram_noinit unsigned int size_find[2];
__mram_noinit request rqst_find[BATCH_SIZE];
__host unsigned int rpse_size = 0;
__mram_noinit response rpse[BATCH_SIZE];

__host mram_allocator *allocator = NULL;
__host dict *dt = NULL;

// __mram request rqst[6] = {
//     {INSERT, 0, {3, "kkk"}, {4, "kkkk"}},
//     {INSERT, 0, {3, "abc"}, {4, "edfg"}},
//     {REPLACE, 0, {3, "kkk"}, {5, "kkkkK"}},
//     {FIND, 0, {3, "kkk"}},
//     {DELETE, 0, {3, "kkk"}},
//     {FIND, 0, {3, "kkk"}}};

int main()
{
    // printf("%d %d\n", size[0], size_find[0]);
    if (!(allocator && dt))
    {
        allocator = mem_alloc(sizeof(mram_allocator));
        dt = mem_alloc(sizeof(dict));
        mram_allocator_initial(allocator);
        dictInit(dt, allocator);
    }
    int ret;
    __mram_ptr dictEntry *he;

    for (int i = 0; i < size[0] + size_find[0]; i++)
    {
        // printf("%d\n", i);
        switch (rqst[i].operate)
        {
        case INSERT:
            ret = dictAdd(dt, rqst[i].key.buf, rqst[i].key.len, rqst[i].val.val, rqst[i].bucket);
            if (ret == DICT_OK)
                rpse[i].response = DICT_ADD_OK;
            else
                rpse[i].response = DICT_ADD_ERR;
            break;
        case FIND:
            he = dictFind(dt, rqst[i].key.buf, rqst[i].key.len, rqst[i].bucket);
            if (he)
            {
                rpse[i].response = DICT_FIND_OK;
                rpse[i].val.val = he->val;
            }
            else
                rpse[i].response = DICT_FIND_ERR;
            break;
//     //     case DELETE:
//     //         ret = dictDelete(dt, rqst[i].key.buf, rqst[i].key.len, rqst[i].bucket);
//     //         if (ret == DICT_OK)
//     //             rpse[i].response = DICT_DELETE_OK;
//     //         else
//     //             rpse[i].response = DICT_DELETE_ERR;
//     //         break;
//     //     case REPLACE:
//     //         ret = dictReplace(dt, rqst[2].key.buf, rqst[2].key.len, rqst[2].val.val, rqst[2].bucket);
//     //         if (ret == DICT_OK)
//     //             rpse[i].response = DICT_REPLACE_OK;
//     //         else
//     //             rpse[i].response = DICT_REPLACE_ERR;
//     //         break;
        }
    }
//     // rpse_size = rqst_size;
//     // // int ret;
//     // // ret = dictAdd(dt, rqst[0].key.buf, rqst[0].key.len, rqst[0].val.buf, rqst[0].val.len, rqst[0].bucket);

//     // // if (ret == DICT_OK)
//     // // {
//     // //     printf("ok!\n");
//     // // }

//     // // ret = dictAdd(dt, rqst[1].key.buf, rqst[1].key.len, rqst[1].val.buf, rqst[1].val.len, rqst[1].bucket);

//     // // if (ret == DICT_OK)
//     // // {
//     // //     printf("ok!\n");
//     // // }

//     // // ret = dictReplace(dt, rqst[2].key.buf, rqst[2].key.len, rqst[2].val.buf, rqst[2].val.len, rqst[2].bucket);

//     // // if (ret == DICT_OK)
//     // // {
//     // //     printf("ok!\n");
//     // // }
//     // // __mram_ptr dictEntry *he;
//     // // he = dictFind(dt, rqst[3].key.buf, rqst[3].key.len, rqst[3].bucket);
//     // // if (he)
//     // // {
//     // //     mram_str_print(he->val);
//     // // }

//     // // ret = dictDelete(dt, rqst[4].key.buf, rqst[4].key.len, rqst[4].bucket);

//     // // if (ret == DICT_OK)
//     // // {
//     // //     printf("ok!\n");
//     // // }

//     // // he = dictFind(dt, rqst[5].key.buf, rqst[5].key.len, rqst[5].bucket);
//     // // if (he)
//     // // {
//     // //     mram_str_print(he->val);
//     // // }
    return 0;
}