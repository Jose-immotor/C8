#ifndef __RECORD_H_
#define __RECORD_H_

#ifdef __cplusplus
extern "C"{
#endif


	
#include "SectorMgr.h"
	
#define RECORD_INVALID_ADD 0xFFFFFFFF

typedef Bool (*RecordVerifyItemFn)(void* pItem, uint32 pParam);

typedef struct _Record
{
	uint32 startAddr;		//��������ʼ��ַ
	uint8  sectorCount;		//��������
	void*  pItem;			//ָ��Ҫ���µ�����
	uint16 itemSize;		//����Item��С
	uint32 total;
	uint8  writeSectorInd;
	uint8  readStartSectorInd;
	uint32 	maxItems;
	int 	itemsPerSec;
	uint32 	readPointer;
	SectorMgr recordMgr;
}Record;

void Record_Seek(Record* pRecord, uint32 pos);
void Record_Init(Record* pRecord, void* pValue, void* pFlashAddr, int itemSize, uint8 sectorCount, RecordVerifyItemFn verify);
Bool Record_Write(Record* pRecord, void* pRecData);
int Record_Read(Record* pRecord, void* pBuf, int buflen);
void Record_Dump(Record* pRecord);
void Record_RemoveAll(Record* pRecord);
uint32 Record_CalcuReadSecAddr(Record* pRecord, uint32 itemInd, uint8* sec, uint32* addrOfSecOffset);
Bool Record_isValid(Record* pRecord);
void Record_ConvertSector(Record* pRecord, uint8* sec, int* ind);

#endif

#ifdef __cplusplus
}
#endif




