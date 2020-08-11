


#ifndef HMAC_H
#define HMAC_H

#ifndef HMAC_C
#define GLOBAL_HMAC extern
#else
#define GLOBAL_HMAC
#endif


GLOBAL_HMAC unsigned char hmac_sha256(const unsigned char * key, unsigned int key_len, 
    const unsigned char * input, unsigned int ilen, 
    unsigned char output [ 32 ]);


#endif

