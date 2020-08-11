
#include "CheckSum.h"

uint16_t CheckSum_Get(uint16_t* pCheckSum, const void* pData, uint8_t len)
{
	const uint8_t* pByte = (uint8_t*)pData;
	uint16_t i = 0;

	for(i=0; i<len; i++)
	{
		*pCheckSum += pByte[i];
	}
	
	return *pCheckSum;
}

