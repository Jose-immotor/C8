/*
 * File      : sectormgr.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-07     lane      first implementation
 */

#include "common.h"
#include "sectormgr.h"
#include "mx25_cmd.h"
#include "nvds.h"

#define CALL_CHECK(expr) if(FlashOperationSuccess != (expr)) {return False;}

void SectorMgr_Dump(SectorMgr* pSectorMgr)
{
	Printf("SectorMgr(@%x) Dump:\n", pSectorMgr);
	
//	Printf("\t m_Version=%d.\n"		, pSectorMgr->m_Version);
	Printf("\t m_startAddr=%d.\n"	, pSectorMgr->m_startAddr);
	Printf("\t m_writeOffset=%d.\n"	, pSectorMgr->m_writeOffset);
	Printf("\t m_readOffset=%d.\n"	, pSectorMgr->m_readOffset);
	
	Printf("\t m_pItem=%x.\n"	, pSectorMgr->m_pItem);
	Printf("\t m_itemSize=%d.\n"	, pSectorMgr->m_itemSize);
	Printf("\t m_ItemCount=%d.\n"	, pSectorMgr->m_ItemCount);
}

Bool SectorMgr_Test()
{
#define BUF_SIZE 512
	int i = 0;
	Bool bRet = False;
	static uint8 tempBuf[BUF_SIZE];
	static uint8 buf1[BUF_SIZE];
	SectorMgr sector[2];

	for(i = 0; i < sizeof(buf1); i++)
	{
		buf1[i]= i;
	}

	SectorMgr_Init(sector, SECTOR_SIZE, 0, tempBuf, sizeof(buf1));

	for(i = 0; i < (SECTOR_SIZE / BUF_SIZE); i++)
	{
		SectorMgr_WriteEx(sector, buf1);
	}

	
	return bRet;
}

//���ݿ��Ƿ�д��
Bool SectorMgr_isDirty(uint8* pByte, int len)
{
	int i = 0;

	for(i = 0; i < len; i++)
	{
		//��������һ����0xFFֵ��˵���������Ѿ���д��
		if(pByte[i] != 0xFF)
		{
			return True;
		}
	}

	return False;
}

void SectorMgr_CalcOffset(SectorMgr* pSector)
{
	static uint8 readBuf[MAX_ITEM_SIZE];//�ŵ�����ᱨӲ������
	uint16 offset = 0;
	uint32 itemsPerSector=0;
	
	itemsPerSector = SECTOR_SIZE/pSector->m_itemSize;

	//�ȼ�����һ��Item�Ƿ��Ѿ���д��
	offset  = (itemsPerSector - 1) * pSector->m_itemSize ;
	Mx25_Read(pSector->m_startAddr + offset, readBuf, pSector->m_itemSize);
	if(SectorMgr_isDirty(readBuf, pSector->m_itemSize))
	{
		pSector->m_writeOffset = 0;
		pSector->m_readOffset  = offset;
		pSector->m_ItemCount   = itemsPerSector;
		return;
	}

	//��0��ʼ, ��λ�հ�λ��
	pSector->m_writeOffset = 0;
	pSector->m_readOffset  = 0;
	pSector->m_ItemCount   = 0;
	
	for(offset = 0; offset <= (SECTOR_SIZE - pSector->m_itemSize * 2); offset += pSector->m_itemSize)
	{
		Mx25_Read(pSector->m_startAddr + offset, readBuf, pSector->m_itemSize);
		
		if(SectorMgr_isDirty(readBuf, pSector->m_itemSize))
		{
			pSector->m_ItemCount++;
			pSector->m_readOffset = offset;
		}
		else
		{
			pSector->m_writeOffset = offset;
			break;
		}
	}
}

//���������в�λд�����ݣ��������Flashĳһλ���𻵣����Զ�����ȥ.
Bool SectorMgr_WriteEx(SectorMgr* pSector, const void* pData)
{
	uint8 startWriteoffset = pSector->m_writeOffset;
	uint8 readBuf[MAX_ITEM_SIZE];
	
	//д����λ����
	while(True)
	{
		if(0 == pSector->m_writeOffset)
		{
			//ִ�в�������
			SectorMgr_Erase(pSector);
		}
		
		//д����
		if(!Mx25_Write(pSector->m_startAddr + pSector->m_writeOffset, (uint8*)pData, pSector->m_itemSize)) 
		{
			return False;
		}
		pSector->m_ItemCount++;

		//�ƶ���ָ��
		pSector->m_readOffset  = pSector->m_writeOffset;
		
		//�ƶ�дָ��,����дλ�õĳ����Ƿ񳬹�SECTOR_SIZE
		pSector->m_writeOffset += pSector->m_itemSize;
		if((pSector->m_writeOffset + pSector->m_itemSize) > SECTOR_SIZE)
		{
			pSector->m_writeOffset = 0;
		}

		//��ȡд�������
		if(!SectorMgr_Read(pSector, readBuf))
		{
			return False;
		}
		
		//ͨ���Ƚ������ж��Ƿ�д�ɹ�,���ʧ�ܣ�˵��Flash������,��д��һ�����顣
		if(memcmp(pData, readBuf, pSector->m_itemSize) == 0)
		{
			//�Ƚϳɹ�
			break;
		}

		//���������ʱ��˵��ȫ�����鶼дʧ�ܣ�Flash���Sectorȫ���𻵡�
		if(startWriteoffset == pSector->m_writeOffset)
		{
			return False;
		}		
	}

	return True;
}

Bool SectorMgr_Write(SectorMgr* pSector)
{
	return SectorMgr_WriteEx(pSector, pSector->m_pItem);
}

//��ȡ����
int SectorMgr_Read(SectorMgr* pSector, void* pBuff)
{
	CALL_CHECK(Mx25_Read(pSector->m_startAddr + pSector->m_readOffset, pBuff, pSector->m_itemSize));

	return pSector->m_itemSize;
}

Bool SectorMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff)
{
	uint32 offset = itemInd * pSector->m_itemSize;

	if(itemInd >= pSector->m_ItemCount)
		return False;	//Offset is invalid.

	CALL_CHECK(Mx25_Read(pSector->m_startAddr + offset, pBuff, pSector->m_itemSize));

	return True;
}

Bool SectorMgr_isFull(SectorMgr* pSector)
{
	return (pSector->m_ItemCount == SECTOR_SIZE / pSector->m_itemSize);
}

Bool SectorMgr_isEmpty(SectorMgr* pSector)
{
	return (pSector->m_ItemCount == 0);
}

Bool SectorMgr_Erase(SectorMgr* pSector)
{
	pSector->m_writeOffset = 0;
	pSector->m_readOffset  = 0;
	pSector->m_ItemCount   = 0;
	return FlashOperationSuccess == Mx25_EraseSector(pSector->m_startAddr);
}

Bool SectorMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc)
{
	uint32 offset = 0;
	uint8 readBuf[PAGE_SIZE];
	
	if(!SectorMgr_isEmpty(pDst))
	{
		SectorMgr_Erase(pDst);
	}

	for(offset = 0; offset < SECTOR_SIZE; offset += PAGE_SIZE)
	{
		Mx25_Read	 (pSrc->m_startAddr + offset, readBuf, PAGE_SIZE);
		Mx25_WritePage(pDst->m_startAddr + offset, readBuf, PAGE_SIZE);
		//CALL_CHECK(Mx25_Read	 (pSrc->m_startAddr + offset, readBuf, PAGE_SIZE));
		//CALL_CHECK(Mx25_WritePage(pDst->m_startAddr + offset, readBuf, PAGE_SIZE));
	}

	SectorMgr_CalcOffset(pDst);
	return True;
}

//��ʼ��һ����������
void SectorMgr_InitFullSector(SectorMgr* pSector, uint32 startAddr, uint16 itemSize)
{
	pSector->m_startAddr = startAddr;
	pSector->m_itemSize	 = itemSize;
	
	pSector->m_writeOffset = 0;
	pSector->m_ItemCount   = SECTOR_SIZE/itemSize;
	pSector->m_readOffset  = (pSector->m_ItemCount - 1) * itemSize ;
}

Bool SectorMgr_Init(SectorMgr* pSector, uint16 sectorSize,
					uint32 startAddr, void* pItem, uint16 itemSize)
{	
	memset(pSector, 0, sizeof(SectorMgr));

	if(itemSize > MAX_ITEM_SIZE) return False;
	
	if(startAddr % sectorSize) return False;	//startAddr����Ҫ��SECTOR_SIZE����
	if(itemSize > sectorSize) return False;	//size����С��SECTOR_SIZE
	
	pSector->m_startAddr = startAddr;
	pSector->m_itemSize	 = itemSize;
	pSector->m_pItem	 = pItem;
	pSector->m_sectorSize = sectorSize;
	
	SectorMgr_CalcOffset(pSector);

	if(pSector->m_ItemCount > 0 && pItem)
	{
		//��ȡ��һ�����ݣ��������ݳ���
		SectorMgr_Read(pSector, pItem);
	}

	//����
	return (pSector->m_ItemCount > 0);
}

