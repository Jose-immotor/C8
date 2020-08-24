#include "common.h"
#include "Record.h"
#include "mx25_cmd.h"
#include "nvds.h"
#include "log.h"
#include "LogUser.h"

#ifdef CFG_LOG

void Record_DumpByCount(Record* pRecord, int count, uint8 moduleId)
{
	int ind = pRecord->itemCount - count;
	if (ind < 0) ind = 0;

	Record_DumpByInd(pRecord, ind, count, moduleId);
}

void Record_DumpByInd(Record* pRecord, int ind, int count, uint8 moduleId)
{
	LogItem item = { 0 };

	Record_Seek(pRecord, ind);

	while (Record_isValid(pRecord) && count--)
	{
		if (Record_Read(pRecord, &item, sizeof(LogItem)))
		{
//			WDT_FEED();
			//����ӡ�ƶ���PMS�ڵ���Ϣ
			if (LogUser_IsPrintf(&item))
			{
				LogUser_Dump(&item, Null, Null);
			}
		}
	}
}

void Record_Dump(Record* pRecord)
{
	Printf("Record Dump:\n");
	
	Printf("\t startAddr=%d.\n"		, pRecord->startAddr);
	Printf("\t sectorCount=%d.\n"	, pRecord->sectorCount);
	Printf("\t total=%d.\n"			, pRecord->itemCount);
	Printf("\t writeSectorInd=%d.\n", pRecord->writeSectorInd);
	Printf("\t readPointer=%d.\n"	, pRecord->readPointer);
	Printf("\t pItem=0x%x.\n"		, pRecord->pItem);
	Printf("\t itemSize=%d.\n"		, pRecord->itemSize);
	SectorMgr_Dump(&pRecord->recordMgr);
}



/*
Bool Record_WritePointerUpdate(Record* pRecord)
{
	uint32 sectorAddr = 0;
	Bool isFull = SectorMgr_isFull(&pRecord->recordMgr);
	
	if(isFull)
	{
		//
		if(pRecord->writeSectorInd + 1 >= pRecord->sectorCount)
		{
			pRecord->writeSectorInd = 0;
			pRecord->itemCount -= pRecord->recordMgr.m_ItemCount;
		}
		else
		{
			pRecord->writeSectorInd++;
		}
		
		Assert(pRecord->itemCount >= pRecord->recordMgr.m_ItemCount);
		
		sectorAddr = pRecord->startAddr + pRecord->writeSectorInd * SECTOR_SIZE;
		Mx25_EraseSector(sectorAddr);
		
	}
	
	return isFull;
}
*/

/*����:ת���������Ϊʵ������
����˵��:
	sec:����ֵ��
	ind:��������Լ�¼λ��
*/
void Record_ConvertSector(Record* pRecord, uint8* sec, int* ind)
{
	int logInd = * ind;
	if(pRecord->readStartSectorInd > *sec)
	{
		logInd -= (pRecord->readStartSectorInd - *sec) * pRecord->itemsPerSec;
	}
	else if(pRecord->readStartSectorInd < *sec)
	{
		logInd += (*sec - pRecord->readStartSectorInd) * pRecord->itemsPerSec;
	}
	*sec = pRecord->readStartSectorInd;

	if(logInd < 0) logInd = 0;
	if(logInd > pRecord->itemCount) logInd = pRecord->itemCount;
	
	*ind = logInd;
}

Bool Record_isValid(Record* pRecord)
{
	return pRecord->itemCount > 0 && pRecord->readPointer < pRecord->itemCount;
}

int Record_GetTotal(Record* pRecord)
{
	return pRecord->itemCount;
}

/*�������ܣ����������ź�������ƫ�Ƶ�ַ
	����:itemInd,Ԫ������λ��.
	����:sec,������.
	����:addrOfSecOffset,�����ڲ�ƫ�Ƶ�ַ.
	����ֵ:���Ե�ַ.
*/
uint32 Record_CalcWriteSecAddr(Record* pRecord, uint32 itemInd, uint8* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	int offsetSec = 0;
	
	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	//��ǰ����������������
	if(pRecord->itemCount == pRecord->maxItems)
	{
		offsetSec = (pRecord->writeSectorInd + 1) % pRecord->sectorCount;
	}
	else if(pRecord->itemCount > pRecord->maxItems - pRecord->itemsPerSec)
	{
		offsetSec = pRecord->writeSectorInd;
	}
	else
	{
		offsetSec = itemInd / pRecord->itemsPerSec;
	}
	
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->itemSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;
	
	return pRecord->startAddr + (offsetSec * SECTOR_SIZE) + addr;
}

Bool Record_Write(Record* pRecord, void* pRecData)
{
	uint32 addrOfSec = 0;
	memcpy(pRecord->pItem, pRecData, pRecord->itemSize);

	if(pRecord->sectorCount == 0) 	//��û�г�ʼ�����
		return False;
	
	if(SectorMgr_Write(&pRecord->recordMgr))
	{
		pRecord->itemCount++;
	}
	if(pRecord->maxItems == pRecord->itemCount)
	{
		pRecord->itemCount = pRecord->maxItems;
	}
	
	uint32 addr = Record_CalcWriteSecAddr(pRecord, pRecord->itemCount % pRecord->maxItems, &pRecord->writeSectorInd, &addrOfSec);
	if(addr == RECORD_INVALID_ADD) return False;
	if(0 == addrOfSec)
	{
		Mx25_EraseSector(addr);
		SectorMgr_Init(&pRecord->recordMgr, SECTOR_SIZE, addr, pRecord->pItem, pRecord->itemSize);
		if(pRecord->itemCount >= pRecord->maxItems)	//������д������
		{
			pRecord->itemCount -= pRecord->itemsPerSec;
			//��ָ��Ҫ����Ӧ�ļ���
			pRecord->readPointer = (pRecord->readPointer > pRecord->itemsPerSec) ? (pRecord->readPointer-pRecord->itemsPerSec) : 0;
		}
		
		//���¶���ʼ������
		Record_CalcuReadSecAddr(pRecord, 0, &pRecord->readStartSectorInd, Null);
	}
	return True;
}

/*�������ܣ����������ź�������ƫ�Ƶ�ַ
	����:itemInd,Ԫ������λ��.
	����:sec,������.
	����:addrOfSecOffset,�����ڲ�ƫ�Ƶ�ַ.
	����ֵ:���Ե�ַ.
*/
uint32 Record_CalcuReadSecAddr(Record* pRecord, uint32 itemInd, uint8* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	uint8 offsetSec = 0;
	
	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	if(pRecord->itemCount >= pRecord->maxItems - pRecord->itemsPerSec)	//ȫ�����������ݶ�����Ч��
	{
		offsetSec = pRecord->writeSectorInd + 1;	//��ָ��Ϊ��дָ�����һ������
	}
	else
	{
		offsetSec = 0;
	}
	
	offsetSec = (offsetSec + itemInd / pRecord->itemsPerSec) % pRecord->sectorCount;
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->itemSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;

	return pRecord->startAddr + (offsetSec * SECTOR_SIZE) + addr;
}

void Record_Seek(Record* pRecord, uint32 pos)
{
	pRecord->readPointer = (pos > pRecord->itemCount) ? pRecord->itemCount : pos;
}

int Record_Read(Record* pRecord, void* pBuf, int buflen)
{
	int i = 0;
	int readLen = 0;
	uint8* pByte = (uint8*)pBuf;
	uint32 addr = 0;

	for(i = pRecord->readPointer
		; i < pRecord->itemCount && buflen >= pRecord->itemSize
		; i++)
	{
		addr = Record_CalcuReadSecAddr(pRecord, i, Null, Null);
		if(addr == RECORD_INVALID_ADD) break;
		
		Mx25_Read(addr, &pByte[readLen], pRecord->itemSize);
		readLen += pRecord->itemSize;
		buflen-= pRecord->itemSize;
	}

	pRecord->readPointer = i;
	return readLen;
}

void Record_RemoveAll(Record* pRecord)
{
	int i = 0;
	if(pRecord->sectorCount == 0) 	//��û�г�ʼ�����
		return;

	//������������
	for(i = 0; i < pRecord->sectorCount; i++)
	{
		Mx25_EraseSector(pRecord->startAddr + i * SECTOR_SIZE);
	}

	//���³�ʼ������������
	pRecord->writeSectorInd = 0;
	pRecord->itemCount = 0;
	pRecord->readPointer = 0;
	pRecord->readStartSectorInd = 0;
	SectorMgr_Init(&pRecord->recordMgr, SECTOR_SIZE, pRecord->startAddr, pRecord->pItem, pRecord->itemSize);
}

void Record_InitSectorMgr(SectorMgr* pSector, uint32 startAddr, void* pItem,
								uint16 itemSize, RecordVerifyItemFn verify)
{
	Bool bRet = True;
	uint8 temp[32];
	
	Assert(itemSize < sizeof(temp));
	
	//����Ĭ��ֵ
	memcpy(temp,  pItem, itemSize);
	
	//��Flash��ȡ����ֵ
	bRet = SectorMgr_Init(pSector, SECTOR_SIZE, startAddr, pItem , itemSize);
	
	if(bRet && !verify(pItem, 0))	//У��汾��
	{
		//Printf("Ver is not match.\n");
		SectorMgr_Erase(pSector);
		//�ָ�Ĭ��ֵ
		memcpy(pItem, temp, itemSize);
	}
}

void Record_Init(Record* pRecord, void* pValue, void* pFlashAddr, 
			int itemSize, uint8 sectorCount, RecordVerifyItemFn verify)
{
	int i = 0;
	int writeSecInd = -1;
	Assert((uint32)pFlashAddr % SECTOR_SIZE == 0);

	memset(pRecord, 0, sizeof(Record));

	pRecord->itemsPerSec = SECTOR_SIZE/itemSize;
	pRecord->maxItems = sectorCount * pRecord->itemsPerSec;

	pRecord->startAddr = (uint32)pFlashAddr;
	pRecord->pItem 	   = pValue;
	pRecord->itemSize  = itemSize;
	pRecord->sectorCount = sectorCount;

	//��������������ͳ����Ч�ļ�¼����, �ҳ���д������
	for(i = 0; i < sectorCount; i++)
	{
		uint32 addr = pRecord->startAddr + i * SECTOR_SIZE;
		Record_InitSectorMgr(&pRecord->recordMgr, addr, pRecord->pItem, 
								pRecord->itemSize, verify);
		//����м���һ������û��д������֮�������������Ҫ��ȫ����Ҫ��ȫ��
		if(!SectorMgr_isFull(&pRecord->recordMgr))
		{		
			if(writeSecInd < 0)	//
			{
				writeSecInd = i;
			}
			else if(pRecord->recordMgr.m_ItemCount > 0)	//��������Ԥ�ڵĴ���ɾ���������־���ݡ�
			{
				for(int j =  writeSecInd + 1; j < sectorCount; j++)
				{
					addr = pRecord->startAddr + j * SECTOR_SIZE;
					Mx25_EraseSector(addr);
				}
				goto End;
			}
		}
		pRecord->itemCount += pRecord->recordMgr.m_ItemCount;
	}
	
	//�Ҳ�����д����
	if(writeSecInd == -1) 
	{
		writeSecInd = 0;
		if(pRecord->itemCount)	//��������Ԥ�ڵĴ���
		{
			Record_RemoveAll(pRecord);
		}
	}
	End:
	pRecord->writeSectorInd = writeSecInd;
	Record_InitSectorMgr(&pRecord->recordMgr,pRecord->startAddr+pRecord->writeSectorInd*SECTOR_SIZE,
							pRecord->pItem, pRecord->itemSize,verify);
	Record_CalcuReadSecAddr(pRecord, 0, &pRecord->readStartSectorInd, Null);
}

#endif
