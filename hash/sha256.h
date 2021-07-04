#ifndef BREE_SHA256_H
#define BREE_SHA256_H
#include <stdint.h>
#define SHA256_HASH_SIZE  32
typedef uint8_t sha256_hash[SHA256_HASH_SIZE];
typedef struct
{
    uint64_t    length;
    uint32_t    state[8];
    uint32_t    curlen;
    uint8_t     buf[64];
} sha256_ctx;

void sha256_init(sha256_ctx& a_ctx);
void sha256_update(sha256_ctx* a_ctx,const uint8_t*a_data,uint32_t a_data_size );
void sha256_finalize(sha256_ctx* a_ctx,uint8_t* a_bytes);
void sha256(const uint8_t*a_data,uint32_t a_data_size,uint8_t* a_bytes);

#endif // BREE_SHA256_H

