#ifndef __CHECK_SUM__H_
#define __CHECK_SUM__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

uint16_t CheckSum_Get(uint16_t* pCheckSum, const void* pData, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif


