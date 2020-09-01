#ifndef __TLV_RX_H_
#define __TLV_RX_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Tlv.h"

typedef enum _TlvInEvent
{
	TE_CHANGED_BEFORE,	//TlvInMgr_updateStroage函数触发外部TLV改变storage值之前,
	TE_CHANGED_AFTER,	//TlvInMgr_updateStroage函数触发外部TLV改变storage值之后
	TE_UPDATE_DONE,		//更新结束，无论storage是否发生改变。
}TlvInEvent;

typedef enum _TlvInEventRc
{
	TERC_SUCCESS = 0,	//返回成功
	TERC_FAILED,		//返回失败
}TlvInEventRc;

struct _TlvIn;
typedef TlvInEventRc(*TlvInEventFn)(void* pObj, const struct _TlvIn* pItem, TlvInEvent ev);
typedef struct _TlvIn
{
	const char* name;	//Name of TLV
	uint32 tag;			//Tag of TLV
	uint32 len;			//Len of TLV
	uint8* storage;		//storage data of TLV
	TlvInEventFn Event;	//function of changed.
}TlvIn;

void TlvInMgr_dump(const TlvIn* pItem);

//Tlv 管理器
typedef struct _TlvInMgr
{
	const TlvIn* itemArray;
	int itemCount;

	uint8_t tagLen;

	void* pObj;
	TlvInEventFn Event;
}TlvInMgr;

void TlvInMgr_init(TlvInMgr* mgr, const TlvIn* items, int itemCount, uint8 tagLen, TlvInEventFn Event);
void TlvInMgr_updateStroage(TlvInMgr* mgr, const uint8* pTlvBuf, int bufSize);
void TlvInMgr_resetAll(TlvInMgr* mgr);

#ifdef __cplusplus
}
#endif

#endif

