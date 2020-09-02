/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "Archdef.h"
#include "SectorMgr.h"

void SectorMgr_Dump(const SectorMgr* pSectorMgr)
{
	Printf("SectorMgr(@%x) Dump:\n", pSectorMgr);
	
//	Printf("\t cfg->Version=%d.\n"		, pSectorMgr->cfg->Version);
	Printf("\t cfg->startAddr=%d.\n"	, pSectorMgr->cfg->startAddr);
	Printf("\t writeOffset=%d.\n"	, pSectorMgr->writeOffset);
	Printf("\t readOffset=%d.\n"	, pSectorMgr->readOffset);
	
	Printf("\t cfg->pItem=%x.\n"	, pSectorMgr->cfg->storage);
	Printf("\t cfg->itemSize=%d.\n"	, pSectorMgr->cfg->storageSize);
	Printf("\t itemCount=%d.\n"	, pSectorMgr->itemCount);
}

Bool SectorMgr_Test()
{
//#define BUF_SIZE 512
//	int i = 0;
//	Bool bRet = False;
//	static uint8 tempBuf[BUF_SIZE];
//	static uint8 buf1[BUF_SIZE];
//	SectorMgr sector[2];
//
//	const SectorCfg cfg =
//	{
//		.startAddr = buf1,
//		.sectorSize = 512,
//		.pItem = 512,
//	}
//
//	for(i = 0; i < sizeof(buf1); i++)
//	{
//		buf1[i]= i;
//	}
//
//	SectorMgr_init(sector, 512, 0, tempBuf, sizeof(buf1));
//
//	for(i = 0; i < (sector->cfg->sectorSize / BUF_SIZE); i++)
//	{
//		SectorMgr_WriteEx(sector, buf1);
//	}

	
	return False;
}

//���ݿ��Ƿ�д��
Bool SectorMgr_isWritten(uint8* pByte, int len)
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
	uint16 offset = 0;
	uint8* readBuf = pSector->cfg->exchangeBuf;
	uint32 itemsPerSector = pSector->cfg->sectorSize / pSector->cfg->storageSize;

	//�ȼ�����һ��Item�Ƿ��Ѿ���д��
	offset = (itemsPerSector - 1) * pSector->cfg->storageSize;
	pSector->cfg->Read(pSector->cfg->startAddr + offset, readBuf, pSector->cfg->storageSize);
	if (SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
	{
		pSector->writeOffset = 0;
		pSector->readOffset = offset;
		pSector->itemCount = itemsPerSector;
		return;
	}

	//����һ��λ���Ƿ��Ѿ���д��
	pSector->cfg->Read(pSector->cfg->startAddr, readBuf, pSector->cfg->storageSize);
	if(!SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
	{
		pSector->writeOffset = 0;
		pSector->readOffset = 0;
		pSector->itemCount = 0;
		return;
	}

	//��λ��д��Χ
	int writeInd = 0;
	int unWriteInd = 0;
	int mid;
	int minInd = 0;
	int maxInd = itemsPerSector;
	for (; (maxInd - minInd) != 1;)
	{
		mid = (maxInd + minInd) >> 1;//mid��n/2��
		pSector->cfg->Read(pSector->cfg->startAddr + mid * pSector->cfg->storageSize, readBuf, pSector->cfg->storageSize);
		if (SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
		{
			minInd = mid;
			writeInd = mid;
		}
		else
		{
			maxInd = mid;
			unWriteInd = mid;
		}
	}

	//�ҳ���дλ��
	writeInd *= pSector->cfg->storageSize;
	unWriteInd *= pSector->cfg->storageSize;
	for (int i = writeInd; i <= unWriteInd; i += pSector->cfg->storageSize)
	{
		pSector->cfg->Read(pSector->cfg->startAddr + i, readBuf, pSector->cfg->storageSize);
		if (!SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
		{
			pSector->readOffset = i - pSector->cfg->storageSize;
			pSector->writeOffset = i;
			pSector->itemCount = i / pSector->cfg->storageSize;
			break;
		}
	}
}

//���������в�λд�����ݣ��������Flashĳһλ���𻵣����Զ�����ȥ.
Bool SectorMgr_WriteEx(SectorMgr* pSector, const void* pData)
{
	uint8 startWriteoffset = pSector->writeOffset;
	uint8 readBuf[MAX_ITEM_SIZE];
	
	//д����λ����
	while(True)
	{
		if(0 == pSector->writeOffset)
		{
			//���������Ƿ�д��������ǣ�ִ�в�������
			for (int i = 0; i < pSector->cfg->sectorSize; i += MAX_ITEM_SIZE)
			{
				if (pSector->cfg->Read(pSector->cfg->startAddr + i, readBuf, MAX_ITEM_SIZE)
					&& SectorMgr_isWritten(readBuf, MAX_ITEM_SIZE))
				{
					SectorMgr_Erase(pSector);
					break;
				}
			}
		}
		
		//д����
		if(!pSector->cfg->Write(pSector->cfg->startAddr + pSector->writeOffset, (uint8*)pData, pSector->cfg->storageSize)) 
		{
			return False;
		}
		pSector->itemCount++;

		//�ƶ���ָ��
		pSector->readOffset  = pSector->writeOffset;
		
		//�ƶ�дָ��,����дλ�õĳ����Ƿ񳬹�pSector->cfg->sectorSize
		pSector->writeOffset += pSector->cfg->storageSize;
		if((pSector->writeOffset + pSector->cfg->storageSize) > pSector->cfg->sectorSize)
		{
			pSector->writeOffset = 0;
		}

		//��ȡд�������
		if(!SectorMgr_Read(pSector, readBuf))
		{
			return False;
		}
		
		//ͨ���Ƚ������ж��Ƿ�д�ɹ�,���ʧ�ܣ�˵��Flash������,��д��һ�����顣
		if(memcmp(pData, readBuf, pSector->cfg->storageSize) == 0)
		{
			//�Ƚϳɹ�
			break;
		}

		//���������ʱ��˵��ȫ�����鶼дʧ�ܣ�Flash���Sectorȫ���𻵡�
		if(startWriteoffset == pSector->writeOffset)
		{
			return False;
		}		
	}

	return True;
}

Bool SectorMgr_IsChanged(const SectorMgr* pSector)
{
	return memcmp((void*)(pSector->cfg->startAddr + pSector->readOffset), pSector->cfg->storage, pSector->cfg->storageSize) != 0;
}

Bool SectorMgr_Write(SectorMgr* pSector)
{
	return SectorMgr_WriteEx(pSector, pSector->cfg->storage);
}

//��ȡ����
int SectorMgr_Read(SectorMgr* pSector, void* pBuff)
{
	if (!pSector->cfg->Read(pSector->cfg->startAddr + pSector->readOffset, pBuff, pSector->cfg->storageSize)) return False;

	return pSector->cfg->storageSize;
}

Bool SectorMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff)
{
	//uint32 offset = itemInd * pSector->cfg->itemSize;

	if(itemInd >= (int)pSector->itemCount)
		return False;	//Offset is invalid.

	if (!pSector->cfg->Read(pSector->cfg->startAddr + pSector->readOffset, pBuff, pSector->cfg->storageSize)) return False;

	return True;
}

Bool SectorMgr_isFull(SectorMgr* pSector)
{
	return (pSector->itemCount == pSector->cfg->sectorSize / pSector->cfg->storageSize);
}

Bool SectorMgr_isEmpty(SectorMgr* pSector)
{
	return (pSector->itemCount == 0);
}

void SectorMgr_Erase(SectorMgr* pSector)
{
	pSector->writeOffset = 0;
	pSector->readOffset  = 0;
	pSector->itemCount   = 0;
	pSector->cfg->Erase(pSector->cfg->startAddr, pSector->cfg->sectorSize);
}

void SectorMgr_Reset(SectorMgr* pSector)
{
	pSector->writeOffset = 0;
	pSector->readOffset = 0;
	pSector->itemCount = 0;
}

Bool SectorMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc, uint8* pageBuf, int bufLen)
{
	uint32 offset = 0;
	
	if(!SectorMgr_isEmpty(pDst))
	{
		SectorMgr_Erase(pDst);
	}

	for(offset = 0; offset < pDst->cfg->sectorSize; offset += bufLen)
	{
		pSrc->cfg->Read (pSrc->cfg->startAddr + offset, pageBuf, bufLen);
		pDst->cfg->Write(pDst->cfg->startAddr + offset, pageBuf, bufLen);
	}

	SectorMgr_CalcOffset(pDst);
	return True;
}

////��ʼ��һ����������
//void SectorMgr_initFullSector(SectorMgr* pSector, uint32 startAddr, uint16 itemSize)
//{
//	pSector->cfg->cfg->startAddr = startAddr;
//	pSector->cfg->cfg->itemSize	 = itemSize;
//	
//	pSector->writeOffset = 0;
//	pSector->itemCount   = pSector->cfg->cfg->sectorSize/itemSize;
//	pSector->readOffset  = (pSector->itemCount - 1) * itemSize ;
//}

Bool SectorMgr_init(SectorMgr* pSector, const SectorCfg* cfg)
{	
	memset(pSector, 0, sizeof(SectorMgr));
	pSector -> cfg = cfg;
	
	if(cfg->startAddr % cfg->sectorSize) return False;	//startAddr����Ҫ��pSector->cfg->sectorSize����
	if(cfg->storageSize > cfg->sectorSize) return False;	//dataLength����С��pSector->cfg->sectorSize
	
	SectorMgr_CalcOffset(pSector);
	Assert(cfg->exchangeBufSize >= cfg->storageSize);

	if(pSector->itemCount > 0 && cfg->storage)
	{
		//��ȡ��һ�����ݣ����������Ƿ�Ϸ�
		if (SectorMgr_Read(pSector, cfg->exchangeBuf))
		{
			if (cfg->Verify && !cfg->Verify(cfg->exchangeBuf, cfg->storageSize))
			{
				SectorMgr_Erase(pSector);
				return False;
			}
			memcpy(cfg->storage, cfg->exchangeBuf, cfg->storageSize);
			return True;
		}
	}

	//����
	return (pSector->itemCount > 0);
}

