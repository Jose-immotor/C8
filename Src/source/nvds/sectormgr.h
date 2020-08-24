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
	uint32 m_startAddr;		//扇区的起始地址
	void*  m_pItem;			//指向要更新的数据
	uint16 m_itemSize;		//扇区Item大小
	uint16 m_sectorSize;	//来自初始化函数，扇区大小
	
	uint16 m_writeOffset;	//扇区内的写偏移地址
	uint16 m_readOffset;	//扇区内的读偏移地址
	uint16 m_ItemCount;		//Item数量
}SectorMgr;

Bool SectorMgr_Init(SectorMgr* pSector, uint16 sectorSize, uint32 startAddr, void* pItem, uint16 itemSize);
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

