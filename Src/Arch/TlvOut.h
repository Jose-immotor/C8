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
	DType dt;
	void* mirror;		//mirror of pStorage, user for compare with storage to confirm if storage is changes.
	TlvIsChangedFn IsChanged;	//function of changed.

	/******************************************
	���һ��TAG,��Ӧ��������value������value�б�Ȼ���������ID��Ϣ��ͨ��TAG+ID�ҵ���Ӧ��ΨһTLV�
	******************************************/
	uint8 idInd;	//id index in storage, ID��Ϣ��storage�е���ʼλ��
	uint8 idLen;	//ID��Ϣ�ĳ���,���Ϊ0����ʾTAGΨһ��Ӧһ��storage
}TlvOut;


//Tlv ������
typedef struct _TlvOutMgr
{
	const TlvOut* itemArray;
	int itemCount;

	uint8 tagLen;

	//���Գ�ʼ������
	//���ΪTrue�����ڽ��պͷ���TLVʱ������Value��DTΪ16λ������32λ�����������Զ����д�С��ת��.
	Bool isSwap;	
}TlvOutMgr;

void TlvOut_dump(const TlvOut* pItem, int tagLen, DType dt);

void TlvOutMgr_init(TlvOutMgr* mgr, const TlvOut* items, int itemCount, int tagLen, Bool isSwap);
void TlvOutMgr_updateMirror(TlvOutMgr* mgr, const uint8* pTlvBuf, int bufSize);

/******************************************
�������ܣ���ȡ�����仯��TLV��
����������
	mgr��Tlv�������ָ�롣
	pBuf�������������
	bufSize�������������С��
	tlvCount���������������
		��Ϊ���������> 0,��ʾ��ȡָ��������TLV��=0��ʾ��ָ��TLV�ĸ�����
		��Ϊ�����������ȡ����TLV�ĸ�����
����ֵ���ܵ���Ч��TLV���ݳ���
******************************************/
int TlvOutMgr_getChanged(TlvOutMgr* mgr, uint8* pBuf, int bufSize, uint8* tlvCount);
int TlvOutMgr_getValByTag(TlvOutMgr* mgr,uint8* pBuf ,int bufSize,uint32 tag);
void TlvOutMgr_resetAll(TlvOutMgr* mgr);
void TlvOutMgr_setFlag(TlvOutMgr* mgr, uint32 tag, TlvOutFlag flag);
const TlvOut* TlvOutMgr_find(const TlvOut* pItems, int count, uint32 tag);

#ifdef __cplusplus
}
#endif

#endif

