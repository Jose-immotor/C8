#ifndef __SAFE_BUF__H_
#define __SAFE_BUF__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include <string.h>

typedef struct _BufItem
{
	volatile uint16	m_BufLen;
	uint8*  m_pBuf;
}BufItem;

typedef struct _SafeBuf
{
	volatile uint16 m_WriteInd;
	volatile uint16 m_ReadIndx;
	
	uint16 m_MaxSize;
	uint8*  m_pBuf;
}SafeBuf;

void SafeBuf_Init(SafeBuf* pSafeBuf, void* pBuf, uint16 bufSize);
int SafeBuf_WriteByte(SafeBuf* pSafeBuf, uint8 data);
int SafeBuf_Write(SafeBuf* pSafeBuf, const void* pData, uint16 len);
int SafeBuf_Read(SafeBuf* pSafeBuf, void* pBuf, int bufSize);
void SafeBuf_Reset(SafeBuf* pSafeBuf);
int SafeBuf_GetCount(SafeBuf* pSafeBuf);
inline uint8 SafeBuf_LatestByte(SafeBuf* pSafeBuf){return pSafeBuf->m_pBuf[pSafeBuf->m_WriteInd-1];};

#ifdef __cplusplus
}
#endif

#endif


