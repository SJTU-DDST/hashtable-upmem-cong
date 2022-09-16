#include <dpu.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "dict.h"
#include "connect.h"
#ifndef DPU_BINARY
#define DPU_BINARY "./dpu_task"
#endif

int main()
{
    struct dpu_set_t set, dpu;
    uint32_t nr_dpus = NR_DPUS;
    uint32_t each_dpu;
    request_batch *rqst = requestInit();
    response_batch *rpse = responseInit();
    dict *dt = dictInit();
    dictAdd(dt, rqst, "kkk", "kkkk");
    dictFind(dt, rqst, "kkk");
    dictReplace(dt, rqst, "kkk", "KKKK");
    dictFind(dt, rqst, "kkk");
    dictDelete(dt, rqst, "kkk");
    dictFind(dt, rqst, "kkk");

    dictAdd(dt, rqst, "abc", "abcd");
    dictFind(dt, rqst, "abc");
    dictReplace(dt, rqst, "abc", "ABCD");
    dictFind(dt, rqst, "abc");
    dictDelete(dt, rqst, "abc");
    dictFind(dt, rqst, "abc");

    for (int i = 0; i < NR_DPUS; i++)
    {
        printf("%d ", rqst->size[i]);
    }
    printf("\n");

    DPU_ASSERT(dpu_alloc(nr_dpus, NULL, &set));
    DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst->size[each_dpu])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "rqst_size", 0, sizeof(unsigned int), DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, each_dpu)
    {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &(rqst->rqst[each_dpu * BATCH_SIZE])));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "rqst", 0, sizeof(request) * BATCH_SIZE, DPU_XFER_DEFAULT));

    // DPU_ASSERT(dpu_broadcast_to(set, "rqst", 0, rqst, sizeof(request) * BATCH_SIZE, DPU_XFER_DEFAULT));
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
    DPU_FOREACH(set, dpu, each_dpu)
    {
        printf("dpu:[%u]\n", each_dpu);
        for (int i = each_dpu * BATCH_SIZE; i < each_dpu * BATCH_SIZE + rpse->size[each_dpu]; i++)
        {
            if (rpse->rpse[i].response == DICT_ADD_OK)
                dt->used++;
            if (rpse->rpse[i].response == DICT_DELETE_OK)
                dt->used--;
            reponsePrint(&(rpse->rpse[i]));
        }
    }

    DPU_ASSERT(dpu_free(set));
    return 0;
}