#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "dict.h"
#include "connect.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define KEY_NUM (1 << 20)
#define FIND_NUM (1 << 16)

Node *node;
int size[2];

int64_t usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((int64_t)tv.tv_sec) * 1000000) + tv.tv_usec;
}

dict *dt;

void operate(request* rqst, response* rpse, int size)
{
    int ret;
    dictEntry *he;
    for (int i = 0; i < size; i++)
    {
        switch (rqst[i].operate)
        {
        case INSERT:
            ret = dictAddCPU(dt, rqst[i].key.buf, rqst[i].key.len, rqst[i].val.val, rqst[i].bucket);
            if (ret == DICT_OK)
                rpse[i].response = DICT_ADD_OK;
            else
                rpse[i].response = DICT_ADD_ERR;
            break;
        case FIND:
            he = dictFindCPU(dt, rqst[i].key.buf, rqst[i].key.len, rqst[i].bucket);
            if (he)
            {
                rpse[i].response = DICT_FIND_OK;
                rpse[i].val.val = he->val;
            }
            else
                rpse[i].response = DICT_FIND_ERR;
            break;
        }
    }

}

int main()
{
    dt = dictInit();
    request_batch *rqst = requestInit();
    request_batch *rqst_find = requestInit();
    response_batch *rpse = responseInit();
    response_batch *rpse_find = responseInit();

    node = (Node *)malloc(sizeof(Node) * KEY_NUM);
    size[0] = 0;
    int64_t start_time = usec();
    printf("start time %llu\n", start_time);

    srand(time(NULL));
    for (int i = 0; i < KEY_NUM; i++)
    {
        char k[8];
        sprintf(k, "%d", i);
        char v[8];
        // sprintf(v, "%d", rand() % KEY_NUM);
        sprintf(v, "%d", i);

        strcpy(node[i].val, v);
        // printf("key: %s, hashed key: %d, value: %s, val nodeptr at %#llx\n", k, dictGetHashedKey(dt, k), node[i].val, &node[i]);

        dictAdd(dt, rqst, k, (NodePtr)&node[i]);
        size[0]++;
    }

    for (int i = 0; i < FIND_NUM; i++)
    {
        char k[8];
        sprintf(k, "%d", rand() % KEY_NUM);
        dictFind(dt, rqst_find, k);
    } 

    int64_t start_time2 = usec();
    printf("start time 2 %llu\n", start_time2);
    printf("%u\n", rqst->size[0]);


    operate(rqst->rqst, rpse->rpse, rqst->size[0]);
    operate(rqst_find->rqst, rpse_find->rpse, rqst_find->size[0]);

    // for (int i = 0; i < rqst_find->size[0]; i++)
    //     reponsePrint(&(rpse_find->rpse[i]));
    

    int64_t end_time = usec();
    printf("end time %llu\n", end_time);
    printf("elapsed time %llu\n", end_time - start_time2);

    return 0;
}