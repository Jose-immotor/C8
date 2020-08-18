#ifndef __SECTOR_MGR__H_
#define __SECTOR_MGR__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
	
#define MAX_USER_COUNT 	10
#define MAX_ITEM_SIZE 	128

typedef struct _Sector
{
	uint32 m_startAddr;		//��������ʼ��ַ
	
	uint16 m_writeOffset;	//�����ڵ�дƫ�Ƶ�ַ
	uint16 m_readOffset;	//�����ڵĶ�ƫ�Ƶ�ַ
	
	void*  m_pItem;			//ָ��Ҫ���µ�����
	uint16 m_itemSize;		//����Item��С
	uint16 m_ItemCount;		//Item����
}SectorMgr;

Bool SectorMgr_Init(SectorMgr* pSector, uint32 startAddr, void* pItem, uint16 itemSize);
void SectorMgr_InitFullSector(SectorMgr* pSector, uint32 startAddr, uint16 itemSize);
Bool SectorMgr_WriteEx(SectorMgr* pSector, const void* pData);
Bool SectorMgr_Write(SectorMgr* pSector);
int SectorMgr_Read(SectorMgr* pSector, void* pBuff);
void SectorMgr_CalcOffset(SectorMgr* pSector);
Bool SectorMgr_isFull(SectorMgr* pSector);
Bool SectorMgr_isEmpty(SectorMgr* pSector);
Bool SectorMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff);
Bool SectorMgr_Erase(SectorMgr* pSector);
Bool SectorMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc);
void SectorMgr_Dump(SectorMgr* pSectorMgr);

#ifdef __cplusplus
}
#endif

#endif

