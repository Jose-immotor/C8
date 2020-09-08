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
	TE_CHANGED_BEFORE,	//TlvInMgr_updateStorage���������ⲿTLV�ı�storageֵ֮ǰ,
	TE_CHANGED_AFTER,	//TlvInMgr_updateStorage���������ⲿTLV�ı�storageֵ֮��
	TE_UPDATE_DONE,		//���½���������storage�Ƿ����ı䡣
}TlvInEvent;

typedef enum _TlvInEventRc
{
	TERC_SUCCESS = 0,	//���سɹ�
	TERC_FAILED,		//����ʧ��
}TlvInEventRc;

struct _TlvIn;
typedef TlvInEventRc(*TlvInEventFn)(void* pObj, const struct _TlvIn* pItem, TlvInEvent ev);
typedef struct _TlvIn
{
	const char* name;	//Name of TLV
	uint32 tag;			//Tag of TLV��
	uint32 len;			//Len of TLV
	uint8* storage;		//storage data of TLV
	DType  dt;			//data type of storage
	TlvInEventFn Event;	//function of changed.
	uint8* mirror;		//mirror of storage, size = len, Null is invalid,use for init mirror from storage
}TlvIn;

void TlvInMgr_dump(const TlvIn* pItem, int tagLen, DType dt);

//Tlv ������
typedef struct _TlvInMgr
{
	const TlvIn* itemArray;
	int itemCount;

	uint8_t tagLen;

	void* pObj;
	TlvInEventFn Event;

	//���ΪTrue�����ڽ��պͷ���TLVʱ������Value��DTΪ16λ������32λ�����������Զ����д�С��ת��.
	Bool isSwap;	
}TlvInMgr;

void TlvInMgr_init(TlvInMgr* mgr, const TlvIn* items, int itemCount, uint8 tagLen, TlvInEventFn Event, Bool isSwap);
void TlvInMgr_updateStorage(TlvInMgr* mgr, const uint8* pTlvBuf, int bufSize);
void TlvInMgr_resetAll(TlvInMgr* mgr);

#ifdef __cplusplus
}
#endif

#endif

