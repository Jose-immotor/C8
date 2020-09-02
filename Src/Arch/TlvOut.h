/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TLV_OUT_H_
#define __TLV_OUT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Tlv.h"

typedef enum _TlvOutFlag
{
	TF_OUT_EMPTY = 0,
	TF_OUT_CHANGED = BIT_0,		//强制改变，触发一次发送动作
}TlvOutFlag;

typedef struct _TlvOutEx
{
	uint32 ticks;		//上次改变的时间Ticks
	uint8  timeOutSec;	//超时计数器, 单位：1S
	TlvOutFlag  flag;		//TF_OUT_CHANGED
}TlvOutEx;

typedef enum _TlvEvent
{
	TE_CHANGED,			//storage发生改变（和mirror比较）
	TE_UNCHANGED,		//storage没发生改变（和mirror比较）
}TlvEvent;

struct _TlvOut;
typedef Bool(*TlvIsChangedFn)(void* pObj, const struct _TlvOut* pItem, TlvEvent ev);
typedef struct _TlvOut
{
	const char* name;	//Name of TLV
	TlvOutEx* pEx;	//extern of TLV

	uint32 tag;			//Tag of TLV
	uint32 len;			//Len of TLV
	void* storage;		//storage data of TLV

	void* mirror;		//mirror of pStorage, user for compare with storage to confirm if storage is changes.
	TlvIsChangedFn IsChanged;	//function of changed.

	/******************************************
	如果一个TAG,对应多个对象的value，则在value中必然包含对象的ID信息。通过TAG+ID找到相应的唯一TLV项。
	******************************************/
	uint8 idInd;	//id index in storage, ID信息在storage中的起始位置
	uint8 idLen;	//ID信息的长度,如果为0，表示TAG唯一对应一个storage
}TlvOut;

void TlvOut_dump(const TlvOut* pItem);

//Tlv 管理器
typedef struct _TlvOutMgr
{
	const TlvOut* itemArray;
	int itemCount;

	uint8 tagLen;
}TlvOutMgr;

void TlvOutMgr_init(TlvOutMgr* mgr, const TlvOut* items, int itemCount, int tagLen);
void TlvOutMgr_updateMirror(TlvOutMgr* mgr, const uint8* pTlvBuf, int bufSize);
int TlvOutMgr_getChanged(TlvOutMgr* mgr, uint8* pBuf, int bufSize);
void TlvOutMgr_resetAll(TlvOutMgr* mgr);

#ifdef __cplusplus
}
#endif

#endif

