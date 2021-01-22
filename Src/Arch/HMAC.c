
#define HMAC_C
#include <string.h>

#include "sha256.h"
#include "HMAC.h"

#define SHA256_BLOCKSIZE 64
unsigned char hmac_sha256(const unsigned char * key, unsigned int key_len, 
    const unsigned char * input, unsigned int ilen, 
    unsigned char output [ 32 ])
#if 1
{
    //
    unsigned char vl_key[SHA256_BLOCKSIZE];
    unsigned char *vl_digest;
    unsigned char i;

    int ret;
    mbedtls_sha256_context ctx;

    vl_digest = (unsigned char *)output;

    memset(vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {
        //mbedtls_sha256((const unsigned char * )key, key_len, vl_key, 0);

        mbedtls_sha256_init( &ctx );
        if( ( ret = mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
            return 0;

        if( ( ret = mbedtls_sha256_update_ret( &ctx, key, key_len ) ) != 0 )
            return 0;

        if( ( ret = mbedtls_sha256_finish_ret( &ctx, vl_key ) ) != 0 )
            return 0;
    }
    else
    {
        memcpy(vl_key, key, key_len);
    }
    // return hash(o_key_pad ［ hash(i_key_pad ［ message))

    mbedtls_sha256_init( &ctx );
    if( ( ret = mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
        return 0;

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        vl_key[i] = 0x36^vl_key[i];
        
    }
    if( ( ret = mbedtls_sha256_update_ret( &ctx, vl_key, SHA256_BLOCKSIZE ) ) != 0 )
            return 0;
    if( ( ret = mbedtls_sha256_update_ret( &ctx, input, ilen ) ) != 0 )
        return 0;

    if( ( ret = mbedtls_sha256_finish_ret( &ctx, vl_digest ) ) != 0 )
        return 0;


    memset(vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {

        mbedtls_sha256_init( &ctx );
        if( ( ret = mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
            return 0;

        if( ( ret = mbedtls_sha256_update_ret( &ctx, key, key_len ) ) != 0 )
            return 0;

        if( ( ret = mbedtls_sha256_finish_ret( &ctx, vl_key ) ) != 0 )
            return 0;
    }
    else
    {
        memcpy(vl_key, key, key_len);
    }

    mbedtls_sha256_init( &ctx );
    if( ( ret = mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
        return 0;

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        vl_key[i] = 0x5C^vl_key[i];
        
    }
    if( ( ret = mbedtls_sha256_update_ret( &ctx, vl_key, SHA256_BLOCKSIZE ) ) != 0 )
        return 0;
    
    if( ( ret = mbedtls_sha256_update_ret( &ctx, vl_digest, 32 ) ) != 0 )
            return 0;

    if( ( ret = mbedtls_sha256_finish_ret( &ctx, vl_digest ) ) != 0 )
        return 0;
    

    return 1;
}

#else
{
    //
    unsigned char vl_key[SHA256_BLOCKSIZE];
    unsigned char o_key_pad[SHA256_BLOCKSIZE*2];
    unsigned char i_key_pad[SHA256_BLOCKSIZE*2];
    unsigned char i;

    memset((char*)vl_key, 0, SHA256_BLOCKSIZE);
    if(key_len > SHA256_BLOCKSIZE)
    {
        mbedtls_sha256((const unsigned char * )key, key_len, vl_key, 0);
    }
    else
    {
        memcpy((char*)vl_key, key, key_len);
    }
    

    for(i = 0; i < SHA256_BLOCKSIZE; i++)
    {
        o_key_pad[i] = 0x5C^vl_key[i];
        i_key_pad[i] = 0x36^vl_key[i];
    }
    // return hash(o_key_pad ［ hash(i_key_pad ［ message))
    memcpy(&i_key_pad[SHA256_BLOCKSIZE], input, ilen);
    mbedtls_sha256((const unsigned char * )i_key_pad, SHA256_BLOCKSIZE + ilen, vl_key, 0);
    memcpy(&o_key_pad[SHA256_BLOCKSIZE], vl_key, 32);
    mbedtls_sha256((const unsigned char * )o_key_pad, SHA256_BLOCKSIZE+32, vl_key, 0);
    memcpy(output, vl_key, 32);
    return 1;
}
#endif
