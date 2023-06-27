#include "host_hash_for_three_index.h"

int huayifan_hash_host_for_primary_key(char *key)//在host端的哈希，用于将主键均匀分散到所有dpu中
{
    return atoi(key)%NR_DPUS;
}

int huayifan_hash_host_for_foreign_key(char *key)//在host端的哈希，用于将主键均匀分散到所有dpu中
{
    return atoi(key)%NR_DPUS;
}

int huayifan_hash_host_for_skiplist(char *key)//在host端的哈希，用于将主键均匀分散到所有dpu中
{
    return atoi(key)%NR_DPUS;
}