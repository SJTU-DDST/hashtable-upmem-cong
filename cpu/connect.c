#include "connect.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void requestReset(request_batch *rqst)
{
    rqst->size[0] = 0;
}
void responseReset(response_batch *rpse)
{
    rpse->size[0] = 0;
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

int requestAdd(request_batch *rqst, unsigned int operate,
               unsigned int bucket, const char *key, const NodePtr val)
{
    rqst->rqst[rqst->size[0]].operate = operate;
    rqst->rqst[rqst->size[0]].bucket = bucket;
    if (key)
    {
        rqst->rqst[rqst->size[0]].key.len = strlen(key);
        strcpy(rqst->rqst[rqst->size[0]].key.buf, key);
    }
    if (val)
    {
        rqst->rqst[rqst->size[0]].val.val = val;
    }
    rqst->size[0]++;
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
        printf("val addr: %#llx, node content: %s\n", rpse->val.val, ((Node*)(rpse->val.val))->val);
        break;
    case DICT_FIND_ERR:
        printf("NOT FIND!\n");
        break;
    default:
        printf("unknown response!\n");
    }
}