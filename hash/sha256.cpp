#include "sha256.h"
#include <memory.h>


static const uint32_t C[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
    0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
    0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
    0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
    0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
    0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
    0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
    0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
    0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
#define BLOCK_SIZE  64
#define MIN(x, y) ( ((x)<(y))?(x):(y) )
#define store_32(x, y)                                                         \
     { (y)[0] = (uint8_t)(((x)>>24)&255); (y)[1] = (uint8_t)(((x)>>16)&255);   \
       (y)[2] = (uint8_t)(((x)>>8)&255); (y)[3] = (uint8_t)((x)&255); }
#define load_32(x, y)                       \
     { x = ((uint32_t)((y)[0] & 255)<<24) | \
           ((uint32_t)((y)[1] & 255)<<16) | \
           ((uint32_t)((y)[2] & 255)<<8)  | \
           ((uint32_t)((y)[3] & 255)); }
#define store_64(x, y)                                                         \
   { (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);     \
     (y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);     \
     (y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);     \
     (y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }

#define Ch( x, y, z )     (z ^ (x & (y ^ z)))
#define M( x, y, z )    (((x | y) & z) | (x & y))
#define cbo(value, bits) (((value) >> (bits)) | ((value) << (32 - (bits))))
#define S( x, n )         cbo((x),(n))
#define R( x, n )         (((x)&0xFFFFFFFFUL)>>(n))
#define G0( x )       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define G1( x )       (S(x, 17) ^ S(x, 19) ^ R(x, 10))
#define S0( x )       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define S1( x )       (S(x, 6) ^ S(x, 11) ^ S(x, 25))

#define sha256_round( a, b, c, d, e, f, g, h, i )   \
     t0 = h + S1(e) + Ch(e, f, g) + C[i] + W[i];\
     t1 = S0(a) + M(a, b, c);                 \
     d += t0;                                       \
     h  = t0 + t1;

#define sha256_round_loop(S,i)\
   sha256_round( S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], i );\
        t = S[7];\
        S[7] = S[6];\
        S[6] = S[5];\
        S[5] = S[4];\
        S[4] = S[3];\
        S[3] = S[2];\
        S[2] = S[1];\
        S[1] = S[0];\
        S[0] = t;

static void sha256_transform(sha256_ctx*a_ctx,uint8_t const*a_data)
{
    uint32_t    t;
    uint32_t    t0;
    uint32_t    t1;
    uint32_t    S[8];
    uint32_t    W[64];



        S[0] = a_ctx->state[0];
        S[1] = a_ctx->state[1];
        S[2] = a_ctx->state[2];
        S[3] = a_ctx->state[3];
        S[4] = a_ctx->state[4];
        S[5] = a_ctx->state[5];
        S[6] = a_ctx->state[6];
        S[7] = a_ctx->state[7];

        load_32( W[0], a_data);
        load_32( W[1], a_data + (4*1) );
        load_32( W[2], a_data + (4*2) );
        load_32( W[3], a_data + (4*3) );
        load_32( W[4], a_data + (4*4) );
        load_32( W[5], a_data + (4*5) );
        load_32( W[6], a_data + (4*6) );
        load_32( W[7], a_data + (4*7) );
        load_32( W[8], a_data + (4*8) );
        load_32( W[9], a_data + (4*9) );
        load_32( W[10], a_data + (4*10) );
        load_32( W[11], a_data + (4*11) );
        load_32( W[12], a_data + (4*12) );
        load_32( W[13], a_data + (4*13) );
        load_32( W[14], a_data + (4*14) );
        load_32( W[15], a_data + (4*15) );


        W[16] = G1( W[14]) + W[9] + G0( W[1] ) + W[0];
        W[17] = G1( W[15]) + W[10] + G0( W[2] ) + W[1];
        W[18] = G1( W[16]) + W[11] + G0( W[3] ) + W[2];
        W[19] = G1( W[17]) + W[12] + G0( W[4] ) + W[3];
        W[20] = G1( W[18]) + W[13] + G0( W[5] ) + W[4];
        W[21] = G1( W[19]) + W[14] + G0( W[6] ) + W[5];
        W[22] = G1( W[20]) + W[15] + G0( W[7] ) + W[6];
        W[23] = G1( W[21]) + W[16] + G0( W[8] ) + W[7];
        W[24] = G1( W[22]) + W[17] + G0( W[9] ) + W[8];
        W[25] = G1( W[23]) + W[18] + G0( W[10] ) + W[9];
        W[26] = G1( W[24]) + W[19] + G0( W[11] ) + W[10];
        W[27] = G1( W[25]) + W[20] + G0( W[12] ) + W[11];
        W[28] = G1( W[26]) + W[21] + G0( W[13] ) + W[12];
        W[29] = G1( W[27]) + W[22] + G0( W[14] ) + W[13];
        W[30] = G1( W[28]) + W[23] + G0( W[15] ) + W[14];
        W[31] = G1( W[29]) + W[24] + G0( W[16] ) + W[15];
        W[32] = G1( W[30]) + W[25] + G0( W[17] ) + W[16];
        W[33] = G1( W[31]) + W[26] + G0( W[18] ) + W[17];
        W[34] = G1( W[32]) + W[27] + G0( W[19] ) + W[18];
        W[35] = G1( W[33]) + W[28] + G0( W[20] ) + W[19];
        W[36] = G1( W[34]) + W[29] + G0( W[21] ) + W[20];
        W[37] = G1( W[35]) + W[30] + G0( W[22] ) + W[21];
        W[38] = G1( W[36]) + W[31] + G0( W[23] ) + W[22];
        W[39] = G1( W[37]) + W[32] + G0( W[24] ) + W[23];
        W[40] = G1( W[38]) + W[33] + G0( W[25] ) + W[24];
        W[41] = G1( W[39]) + W[34] + G0( W[26] ) + W[25];
        W[42] = G1( W[40]) + W[35] + G0( W[27] ) + W[26];
        W[43] = G1( W[41]) + W[36] + G0( W[28] ) + W[27];
        W[44] = G1( W[42]) + W[37] + G0( W[29] ) + W[28];
        W[45] = G1( W[43]) + W[38] + G0( W[30] ) + W[29];
        W[46] = G1( W[44]) + W[39] + G0( W[31] ) + W[30];
        W[47] = G1( W[45]) + W[40] + G0( W[32] ) + W[31];
        W[48] = G1( W[46]) + W[41] + G0( W[33] ) + W[32];
        W[49] = G1( W[47]) + W[42] + G0( W[34] ) + W[33];
        W[50] = G1( W[48]) + W[43] + G0( W[35] ) + W[34];
        W[51] = G1( W[49]) + W[44] + G0( W[36] ) + W[35];
        W[52] = G1( W[50]) + W[45] + G0( W[37] ) + W[36];
        W[53] = G1( W[51]) + W[46] + G0( W[38] ) + W[37];
        W[54] = G1( W[52]) + W[47] + G0( W[39] ) + W[38];
        W[55] = G1( W[53]) + W[48] + G0( W[40] ) + W[39];
        W[56] = G1( W[54]) + W[49] + G0( W[41] ) + W[40];
        W[57] = G1( W[55]) + W[50] + G0( W[42] ) + W[41];
        W[58] = G1( W[56]) + W[51] + G0( W[43] ) + W[42];
        W[59] = G1( W[57]) + W[52] + G0( W[44] ) + W[43];
        W[60] = G1( W[58]) + W[53] + G0( W[45] ) + W[44];
        W[61] = G1( W[59]) + W[54] + G0( W[46] ) + W[45];
        W[62] = G1( W[60]) + W[55] + G0( W[47] ) + W[46];
        W[63] = G1( W[61]) + W[56] + G0( W[48] ) + W[47];



        sha256_round_loop(S,0);
        sha256_round_loop(S,1);
        sha256_round_loop(S,2);
        sha256_round_loop(S,3);
        sha256_round_loop(S,4);
        sha256_round_loop(S,5);
        sha256_round_loop(S,6);
        sha256_round_loop(S,7);
        sha256_round_loop(S,8);
        sha256_round_loop(S,9);

        sha256_round_loop(S,10);
        sha256_round_loop(S,11);
        sha256_round_loop(S,12);
        sha256_round_loop(S,13);
        sha256_round_loop(S,14);
        sha256_round_loop(S,15);
        sha256_round_loop(S,16);
        sha256_round_loop(S,17);
        sha256_round_loop(S,18);
        sha256_round_loop(S,19);

        sha256_round_loop(S,20);
        sha256_round_loop(S,21);
        sha256_round_loop(S,22);
        sha256_round_loop(S,23);
        sha256_round_loop(S,24);
        sha256_round_loop(S,25);
        sha256_round_loop(S,26);
        sha256_round_loop(S,27);
        sha256_round_loop(S,28);
        sha256_round_loop(S,29);

        sha256_round_loop(S,30);
        sha256_round_loop(S,31);
        sha256_round_loop(S,32);
        sha256_round_loop(S,33);
        sha256_round_loop(S,34);
        sha256_round_loop(S,35);
        sha256_round_loop(S,36);
        sha256_round_loop(S,37);
        sha256_round_loop(S,38);
        sha256_round_loop(S,39);

        sha256_round_loop(S,40);
        sha256_round_loop(S,41);
        sha256_round_loop(S,42);
        sha256_round_loop(S,43);
        sha256_round_loop(S,44);
        sha256_round_loop(S,45);
        sha256_round_loop(S,46);
        sha256_round_loop(S,47);
        sha256_round_loop(S,48);
        sha256_round_loop(S,49);

        sha256_round_loop(S,50);
        sha256_round_loop(S,51);
        sha256_round_loop(S,52);
        sha256_round_loop(S,53);
        sha256_round_loop(S,54);
        sha256_round_loop(S,55);
        sha256_round_loop(S,56);
        sha256_round_loop(S,57);
        sha256_round_loop(S,58);
        sha256_round_loop(S,59);

        sha256_round_loop(S,60);
        sha256_round_loop(S,61);
        sha256_round_loop(S,62);
        sha256_round_loop(S,63);

        a_ctx->state[0] = a_ctx->state[0] + S[0];
        a_ctx->state[1] = a_ctx->state[1] + S[1];
        a_ctx->state[2] = a_ctx->state[2] + S[2];
        a_ctx->state[3] = a_ctx->state[3] + S[3];
        a_ctx->state[4] = a_ctx->state[4] + S[4];
        a_ctx->state[5] = a_ctx->state[5] + S[5];
        a_ctx->state[6] = a_ctx->state[6] + S[6];
        a_ctx->state[7] = a_ctx->state[7] + S[7];

}

void sha256_init(sha256_ctx& a_ctx)
{
    a_ctx.curlen = 0;
    a_ctx.length = 0;
    a_ctx.state[0] = 0x6A09E667UL;
    a_ctx.state[1] = 0xBB67AE85UL;
    a_ctx.state[2] = 0x3C6EF372UL;
    a_ctx.state[3] = 0xA54FF53AUL;
    a_ctx.state[4] = 0x510E527FUL;
    a_ctx.state[5] = 0x9B05688CUL;
    a_ctx.state[6] = 0x1F83D9ABUL;
    a_ctx.state[7] = 0x5BE0CD19UL;
}

void sha256_update(sha256_ctx* a_ctx,const uint8_t*a_data,uint32_t a_data_size)
{
    uint32_t n;
    if( a_ctx->curlen > sizeof(a_ctx->buf) )
    {
       return;// ?
    }
    while( a_data_size > 0 )
    {
        if( a_ctx->curlen == 0 && a_data_size >= BLOCK_SIZE )
        {
           sha256_transform( a_ctx, (uint8_t*)a_data );
           a_ctx->length += BLOCK_SIZE * 8;
           a_data = (uint8_t*)a_data + BLOCK_SIZE;
           a_data_size -= BLOCK_SIZE;
        }
        else
        {
           n = MIN( a_data_size, (BLOCK_SIZE - a_ctx->curlen) );
           memcpy( a_ctx->buf + a_ctx->curlen, a_data, (size_t)n );
           a_ctx->curlen += n;
           a_data = (uint8_t*)a_data + n;
           a_data_size -= n;
           if( a_ctx->curlen == BLOCK_SIZE )
           {
              sha256_transform( a_ctx, a_ctx->buf );
              a_ctx->length += 8*BLOCK_SIZE;
              a_ctx->curlen = 0;
           }
       }
    }
}
void sha256_finalize(sha256_ctx* a_ctx,uint8_t* a_bytes)//,sha256_hash*Digest)
{

    if( a_ctx->curlen >= sizeof(a_ctx->buf) )
    {
       return;
    }
    a_ctx->length += a_ctx->curlen * 8;

    a_ctx->buf[a_ctx->curlen++] = (uint8_t)0x80;

    if( a_ctx->curlen > 56 )
    {
        while( a_ctx->curlen < 64 )
        {
            a_ctx->buf[a_ctx->curlen++] = (uint8_t)0;
        }
        sha256_transform(a_ctx, a_ctx->buf);
        a_ctx->curlen = 0;
    }
    while( a_ctx->curlen < 56 )
    {
        a_ctx->buf[a_ctx->curlen++] = (uint8_t)0;
    }
    store_64( a_ctx->length, a_ctx->buf+56 );
    sha256_transform( a_ctx, a_ctx->buf );

        store_32( a_ctx->state[0], a_bytes );
        store_32( a_ctx->state[1], a_bytes+(4*1) );
        store_32( a_ctx->state[2], a_bytes+(4*2) );
        store_32( a_ctx->state[3], a_bytes+(4*3) );
        store_32( a_ctx->state[4], a_bytes+(4*4) );
        store_32( a_ctx->state[5], a_bytes+(4*5) );
        store_32( a_ctx->state[6], a_bytes+(4*6) );
        store_32( a_ctx->state[7], a_bytes+(4*7) );

}

void sha256(const uint8_t*a_data,uint32_t a_data_size,uint8_t* a_bytes)
{
    sha256_ctx l_ctx;
    sha256_init( l_ctx );
    sha256_update( &l_ctx, a_data, a_data_size );
    sha256_finalize( &l_ctx, a_bytes );
}
