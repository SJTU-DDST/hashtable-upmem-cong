#include "connect.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
void requestReset(request_batch *rqst)
{
    int i;
    for (i = 0; i < NR_DPUS; i++)
    {
        rqst->size[i] = 0;
    }
}
void responseReset(response_batch *rpse)
{
    int i;
    for (i = 0; i < NR_DPUS; i++)
    {
        rpse->size[i] = 0;
    }
}
request_batch *requestInit()
{
    request_batch *rqst = malloc(sizeof(request_batch));
    requestReset(rqst);
    return rqst;
}
response_batch *responseInit()
{
    response_batch *rpse = malloc(sizeof(response_batch));
    responseReset(rpse);
    return rpse;
}

int requestAdd(request_batch *rqst, unsigned int dpu, unsigned int operate,
               unsigned int bucket, const char *key, const char *val)
{
    if (dpu >= NR_DPUS || rqst->size[dpu] >= BATCH_SIZE)
        return CONNECT_ERR;

    rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].operate = operate;
    rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].bucket = bucket;
    if (key)
    {
        rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].key.len = strlen(key);
        strcpy(rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].key.buf, key);
    }
    if (val)
    {
        rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].val.len = strlen(val);
        strcpy(rqst->rqst[dpu * BATCH_SIZE + rqst->size[dpu]].val.buf, val);
    }
    rqst->size[dpu]++;
    return CONNECT_OK;
}

void reponsePrint(response *rpse)
{
    switch (rpse->response)
    {
    case DICT_ADD_OK:
        printf("ADD succeed!\n");
        break;
    case DICT_ADD_ERR:
        printf("ADD fail!\n");
        break;
    case DICT_REPLACE_OK:
        printf("REPLACE succeed!\n");
        break;
    case DICT_REPLACE_ERR:
        printf("REPLACE fail!\n");
        break;
    case DICT_DELETE_OK:
        printf("DELETE succeed!\n");
        break;
    case DICT_DELETE_ERR:
        printf("DELETE fail!\n");
        break;
    case DICT_FIND_OK:
        printf("FIND succeed!\n");
        printf("val:%s\n", rpse->val.buf);
        break;
    case DICT_FIND_ERR:
        printf("NOT FIND!\n");
        break;
    default:
        printf("unknown response!\n");
    }
}