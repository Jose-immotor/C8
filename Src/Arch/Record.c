
#include "ArchDef.h"
#include "Record.h"
#include "DateTime.h"

#ifdef CFG_LOG

void Record_Dump(Record* pRecord)
{
	Printf("Record Dump:\n");
	
	Printf("\t startAddr=%d.\n"		, pRecord->cfg->base.startAddr);
	Printf("\t sectorCount=%d.\n"	, pRecord->cfg->sectorCount);
	
	Printf("\t total=%d.\n"			, pRecord->itemCount);
	
	Printf("\t writeSectorInd=%d.\n", pRecord->writeSectorInd);
	Printf("\t readPointer=%d.\n"	, pRecord->readPointer);
	
	SectorMgr_Dump(&pRecord->sector);
}

/*����:ת���������Ϊʵ������
����˵��:
	sec:����ֵ��
	ind:��������Լ�¼λ��
*/
void Record_ConvertSector(Record* pRecord, uint16* sec, int* ind)
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
	if(logInd > (int)pRecord->itemCount) logInd = pRecord->itemCount;
	
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
uint32 Record_CalcWriteSecAddr(Record* pRecord, uint32 itemInd, uint16* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	int offsetSec = 0;
//	const RecordCfg* cfg = pRecord->cfg;

	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	//��ǰ����������������
	if(pRecord->itemCount == pRecord->maxItems)
	{
		offsetSec = (pRecord->writeSectorInd + 1) % pRecord->cfg->sectorCount;
	}
	else if(pRecord->itemCount > pRecord->maxItems - pRecord->itemsPerSec)
	{
		offsetSec = pRecord->writeSectorInd;
	}
	else
	{
		offsetSec = itemInd / pRecord->itemsPerSec;
	}
	
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->sector.cfg->storageSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;
	
	return pRecord->cfg->base.startAddr + (offsetSec * pRecord->sector.cfg->sectorSize) + addr;
}

Bool Record_Write(Record* pRecord, void* pRecData)
{
//	const RecordCfg* cfg = pRecord->cfg;
	uint32 addrOfSec = 0;
	memcpy(pRecord->sector.cfg->storage, pRecData, pRecord->sector.cfg->storageSize);

	if(pRecord->cfg->sectorCount == 0) 	//��û�г�ʼ�����
		return False;
	
	if(SectorMgr_Write(&pRecord->sector))
	{
		pRecord->itemCount++;
	}
	
	uint16 sectorInd = pRecord->writeSectorInd;
	uint32 addr = Record_CalcWriteSecAddr(pRecord, pRecord->itemCount % pRecord->maxItems, &pRecord->writeSectorInd, &addrOfSec);
	if(addr == RECORD_INVALID_ADD) return False;
	if (pRecord->writeSectorInd != sectorInd)	//��ǰ����д��,�л�����һ���µ�����
	{
		//�л�����һ���µ�����
		pRecord->sectorCfg.startAddr = addr;
		SectorMgr_Reset(&pRecord->sector);
		if(pRecord->itemCount >= pRecord->maxItems)
		{
			//��������������
			SectorMgr_Erase(&pRecord->sector);
			//������ЧԪ������
			pRecord->itemCount -= pRecord->itemsPerSec;
			//�ƶ���ָ���α꣬�����ǰ��ָ��ɾ����������Ҫ�ƶ�����һ���ɶ�λ��
			pRecord->readPointer = ((int)pRecord->readPointer > pRecord->itemsPerSec) ? (pRecord->readPointer - pRecord->itemsPerSec) : 0;
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
uint32 Record_CalcuReadSecAddr(Record* pRecord, uint32 itemInd, uint16* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	uint8 offsetSec = 0;
//	const RecordCfg* cfg = pRecord->cfg;
	
	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	if(pRecord->itemCount >= pRecord->maxItems - pRecord->itemsPerSec)	//ȫ�����������ݶ�����Ч��
	{
		offsetSec = pRecord->writeSectorInd + 1;	//��ָ��Ϊ��дָ�����һ������
	}
	else
	{
		offsetSec = 0;
	}
	
	offsetSec = (offsetSec + itemInd / pRecord->itemsPerSec) % pRecord->cfg->sectorCount;
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->sector.cfg->storageSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;

	return pRecord->cfg->base.startAddr + (offsetSec * pRecord->sector.cfg->sectorSize) + addr;
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
	uint16 itemSize = pRecord->sector.cfg->storageSize;

	for(i = pRecord->readPointer
		; i < (int)pRecord->itemCount && buflen >= itemSize
		; i++)
	{
		addr = Record_CalcuReadSecAddr(pRecord, i, Null, Null);
		if(addr == RECORD_INVALID_ADD) break;
		
		pRecord->cfg->base.Read(addr, &pByte[readLen], itemSize);
		readLen += itemSize;
		buflen-= itemSize;
	}

	pRecord->readPointer = i;
	return readLen;
}

void Record_RemoveAll(Record* pRecord)
{
	int i = 0;
	if(pRecord->cfg->sectorCount == 0) 	//��û�г�ʼ�����
		return;
	uint16 SECTOR_SIZE = pRecord->sector.cfg->sectorSize;
	//������������
	for(i = 0; i < pRecord->cfg->sectorCount; i++)
	{
		pRecord->cfg->base.Erase(pRecord->cfg->base.startAddr + i * SECTOR_SIZE, SECTOR_SIZE);
	}

	//���³�ʼ������������
	pRecord->writeSectorInd = 0;
	pRecord->itemCount = 0;
	pRecord->readPointer = 0;
	pRecord->readStartSectorInd = 0;
	SectorMgr_Reset(&pRecord->sector);
}

void Record_Init(Record* pRecord, const RecordCfg* cfg)
{
	int i = 0;
	int writeSecInd = -1;
	SectorCfg* secCfg = &pRecord->sectorCfg;

	memset(pRecord, 0, sizeof(Record));
	pRecord->cfg = cfg;
	memcpy(secCfg, &cfg->base, sizeof(SectorCfg));

	Assert((uint32)cfg->base.startAddr % cfg->base.sectorSize == 0);
	pRecord->itemsPerSec = secCfg->sectorSize / secCfg->storageSize;
	pRecord->maxItems = pRecord->cfg->sectorCount * pRecord->itemsPerSec;

	//��������������ͳ����Ч�ļ�¼����, �ҳ���д������
	SectorMgr* pMgr = &pRecord->sector;
	for(i = 0; i < cfg->sectorCount; i++)
	{
		uint32 addr = cfg->base.startAddr + i * secCfg->sectorSize;
		secCfg->startAddr = addr;
		if (!SectorMgr_init(&pRecord->sector, secCfg))
		{
			//�������
			writeSecInd = -1;
			break;
		}

		//����м���һ������û��д������֮�������������Ҫ��ȫ����Ҫ��ȫ��
		if(!SectorMgr_isFull(pMgr))
		{		
			if(writeSecInd < 0)	//��һ��
			{
				writeSecInd = i;
			}
			else if(pMgr->itemCount > 0)	//��������Ԥ�ڵĴ���ɾ���������־���ݡ�
			{
				cfg->base.Erase(addr, cfg->base.sectorSize);
				continue;
			}
		}
		pRecord->itemCount += pMgr->itemCount;
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

	pRecord->writeSectorInd = writeSecInd;
	secCfg->startAddr = pRecord->cfg->base.startAddr + pRecord->writeSectorInd * secCfg->sectorSize;
	SectorMgr_init(&pRecord->sector, &pRecord->sectorCfg);
	Record_CalcuReadSecAddr(pRecord, 0, &pRecord->readStartSectorInd, Null);
}

#endif
