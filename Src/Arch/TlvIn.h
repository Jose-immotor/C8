/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TLV_RX_H_
#define __TLV_RX_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Tlv.h"

typedef enum _TlvInEvent
{
	TE_CHANGED_BEFORE,	//TlvInMgr_updateStorage函数触发外部TLV改变storage值之前,
	TE_CHANGED_AFTER,	//TlvInMgr_updateStorage函数触发外部TLV改变storage值之后
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
	uint32 tag;			//Tag of TLV，
	uint32 len;			//Len of TLV
	uint8* storage;		//storage data of TLV
	DType  dt;			//data type of storage
	TlvInEventFn Event;	//function of changed.
	uint8* mirror;		//mirror of storage, size = len, Null is invalid,use for init mirror from storage
}TlvIn;

void TlvInMgr_dump(const TlvIn* pItem, int tagLen, DType dt);

//Tlv 管理器
typedef struct _TlvInMgr
{
	const TlvIn* itemArray;
	int itemCount;

	uint8_t tagLen;

	void* pObj;
	TlvInEventFn Event;

	//如果为True，则在接收和发送TLV时，对于Value的DT为16位整数和32位整数的类型自动进行大小端转换.
	Bool isSwap;	
}TlvInMgr;

void TlvInMgr_init(TlvInMgr* mgr, const TlvIn* items, int itemCount, uint8 tagLen, TlvInEventFn Event, Bool isSwap);
void TlvInMgr_updateStorage(TlvInMgr* mgr, const uint8* pTlvBuf, int bufSize);
void TlvInMgr_resetAll(TlvInMgr* mgr);

#ifdef __cplusplus
}
#endif

#endif

