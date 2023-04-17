#ifndef __CONNECT_H
#define __CONNECT_H

#define KEY_BUF_SIZE 16
#define VAL_BUF_SIZE 32
#define NR_DPUS 4
#define BATCH_SIZE 32768

#define CONNECT_OK 0
#define CONNECT_ERR 1

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

#include <stddef.h>
typedef struct Node 
{
    char val[10];
} Node;
typedef unsigned long long NodePtr;

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
typedef struct request_batch
{
    unsigned int size[NR_DPUS];
    request rqst[BATCH_SIZE * NR_DPUS];
} request_batch;
typedef struct response_batch
{
    unsigned int size[NR_DPUS];
    response rpse[BATCH_SIZE * NR_DPUS];
} response_batch;

request_batch *requestInit();
response_batch *responseInit();
void requestReset(request_batch *rqst);
void responseReset(response_batch *rpse);
int requestAdd(request_batch *rqst, unsigned int dpu, unsigned int operate, unsigned int bucket, const char *key, const NodePtr val);
void reponsePrint(response* rpse);
#endif /* __DICT_H */