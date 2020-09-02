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
	TF_OUT_CHANGED = BIT_0,		//ǿ�Ƹı䣬����һ�η��Ͷ���
}TlvOutFlag;

typedef struct _TlvOutEx
{
	uint32 ticks;		//�ϴθı��ʱ��Ticks
	uint8  timeOutSec;	//��ʱ������, ��λ��1S
	TlvOutFlag  flag;		//TF_OUT_CHANGED
}TlvOutEx;

typedef enum _TlvEvent
{
	TE_CHANGED,			//storage�����ı䣨��mirror�Ƚϣ�
	TE_UNCHANGED,		//storageû�����ı䣨��mirror�Ƚϣ�
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
	���һ��TAG,��Ӧ��������value������value�б�Ȼ���������ID��Ϣ��ͨ��TAG+ID�ҵ���Ӧ��ΨһTLV�
	******************************************/
	uint8 idInd;	//id index in storage, ID��Ϣ��storage�е���ʼλ��
	uint8 idLen;	//ID��Ϣ�ĳ���,���Ϊ0����ʾTAGΨһ��Ӧһ��storage
}TlvOut;

void TlvOut_dump(const TlvOut* pItem);

//Tlv ������
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

