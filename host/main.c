#include <dpu.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "dict.h"
#include "connect.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#ifndef DPU_BINARY
#define DPU_BINARY "./dpu_task"
#endif

#define KEY_NUM (1 << 16)
#define FIND_NUM (1 << 10)
#define MAX_QUERY_PER_DPU (1024)

Node *node;

int64_t usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((int64_t)tv.tv_sec) * 1000000) + tv.tv_usec;
}

char preprocess_key_buffer[NR_DPUS][MAX_QUERY_PER_DPU][KEY_BUF_SIZE];
NodePtr preprocess_val_buffer[NR_DPUS][MAX_QUERY_PER_DPU];
int size[NR_DPUS];
char preprocess_key_buffer_find[NR_DPUS][MAX_QUERY_PER_DPU][KEY_BUF_SIZE];
int size_find[NR_DPUS];

int main()
{
    struct dpu_set_t set, dpu;
    uint32_t nr_dpus = NR_DPUS;
    uint32_t each_dpu;
    request_batch *rqst = requestInit();
    request_batch *rqst_find = requestInit();
    response_batch *rpse = responseInit();
    // response_batch *rpse_find = responseInit();
    dict *dt = dictInit();

    node = (Node *)malloc(sizeof(Node) * KEY_NUM);
    int64_t start_time = usec();
    printf("start time %llu\n", start_time);

    memset(size, 0, sizeof(size));
    memset(size_find, 0, sizeof(size_find));
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
        int which_dpu = dictGetHashedKey(dt, k) % NR_DPUS;
        strcpy(preprocess_key_buffer[which_dpu][size[which_dpu]], k);
        preprocess_val_buffer[which_dpu][size[which_dpu]] = &node[i];
        size[which_dpu]++;
    }
    int max_size = -1;
    for (int i = 0; i < NR_DPUS; i++)
    {
        // printf("dpu %d: keynum %d\n", i, size[i]);
        max_size = max_size > size[i] ? max_size : size[i];
    }
    for (int i = 0; i < NR_DPUS; i++)
        for (int j = 0; j < size[i]; j++)
            dictAddSpecificDPU(dt, rqst, i, preprocess_key_buffer[i][j], preprocess_val_buffer[i][j], max_size);

    for (int i = 0; i < FIND_NUM; i++)
    {
        char k[8];
        sprintf(k, "%d", rand() % KEY_NUM);
        // dictFind(dt, rqst_find, k);
        int which_dpu = dictGetHashedKey(dt, k) % NR_DPUS;
        strcpy(preprocess_key_buffer_find[which_dpu][size_find[which_dpu]], k);
        size_find[which_dpu]++;
    }
    int max_size_find = -1;
    for (int i = 0; i < NR_DPUS; i++)
    {
        // printf("dpu %d: findkeynum %d\n", i, size_find[i]);
        max_size_find = max_size_find > size_find[i] ? max_size_find : size_find[i];
    }
    for (int i = 0; i < NR_DPUS; i++)
        for (int j = 0; j < size_find[i]; j++)
            dictFindSpecificDPU(dt, rqst_find, i, preprocess_key_buffer[i][j], max_size_find);

    // for (int i = 0; i < NR_DPUS; i++)
    // {
    //     printf("%d ", rqst_find->size[i]);
    // }
    // printf("\n");

    int64_t start_time2 = usec();
    printf("start time 2 %llu\n", start_time2);

    DPU_ASSERT(dpu_alloc(nr_dpus, NULL, &set));
    DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));

    // DPU_FOREACH(set, dpu, each_dpu)
    // {
    //     DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst->size[each_dpu])));
    // }
    // DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "rqst_size", 0, sizeof(unsigned int), DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst->size[each_dpu])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "size", 0, sizeof(unsigned int) * 2, DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst->rqst[each_dpu * max_size])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "rqst", 0, sizeof(request) * max_size, DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst_find->size[each_dpu])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "size_find", 0, sizeof(unsigned int) * 2, DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst_find->rqst[each_dpu * max_size_find])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "rqst_find", 0, sizeof(request) * max_size_find, DPU_XFER_DEFAULT));

    DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rpse->size[each_dpu])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "rpse_size", 0, sizeof(unsigned int), DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rpse->rpse[each_dpu * BATCH_SIZE])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "rpse", 0, sizeof(response) * BATCH_SIZE, DPU_XFER_DEFAULT));

    // DPU_FOREACH(set, dpu)
    // {
    //     DPU_ASSERT(dpu_log_read(dpu, stdout));
    // }


    // DPU_FOREACH(set, dpu, each_dpu)
    // {
    //     printf("dpu:[%u] size: %d\n", each_dpu, rpse->size[each_dpu]);
    //     for (int i = each_dpu * BATCH_SIZE; i < each_dpu * BATCH_SIZE + rpse->size[each_dpu]; i++)
    //     {
    //         if (rpse->rpse[i].response == DICT_ADD_OK)
    //             dt->used++;
    //         if (rpse->rpse[i].response == DICT_DELETE_OK)
    //             dt->used--;
    //         reponsePrint(&(rpse->rpse[i]));
    //     }
    // }

    DPU_ASSERT(dpu_free(set));


    int64_t end_time = usec();
    printf("end time %llu\n", end_time);
    printf("elapsed time %llu\n", end_time - start_time2);

    return 0;
}