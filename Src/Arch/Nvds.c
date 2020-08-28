#include "ArchDef.h"
#include "Nvds.h"

static Nvds g_Nvds;

Bool Nvds_verify(const uint8* pByte, int len)
{
	return (pByte[0] != EEPROM_FIRST_BYTE || pByte[len - 1] != EEPROM_LATEST_BYTE);
}

//����IDдNVDS
void Nvds_write(uint8_t nvdsId)
{
	const NvdsItem* p = g_Nvds.nvdsItemArray;
	for (int i = 0; i < g_Nvds.nvdsItemArrayCount; i++, p++)
	{
		if (p->id == nvdsId)
		{
			Bool isOk = p->Event ? p->Event(p->secCfg.storage, BE_ON_WRITE_BEFORE) : True;
			if (isOk)
			{
				SectorMgr_Write(p->sectorMgr);
			}
			break;
		}
	}
}

void Nvds_InitItem(const NvdsItem* item)
{
	const SectorCfg* cfg = &item->secCfg;
	uint8* pByte = (uint8*)cfg->storage;

	SectorMgr_init(item->sectorMgr, cfg);

	//���������Ƿ���Ч
	if (!Nvds_verify(pByte, cfg->storageSize))
	{
		//������Ч�����µ���Ĭ�ϵĳ�ʼ������
		item->Event(pByte, BE_DATA_ERROR);

		pByte[0] = EEPROM_FIRST_BYTE;
		pByte[cfg->storageSize - 1] = EEPROM_LATEST_BYTE;
		if (item->sectorMgr->itemCount)	//�����д��,��ִ��ɾ������
		{
			SectorMgr_Erase(item->sectorMgr);
		}
	}
	else
	{
		item->Event(pByte, BE_DATA_OK);
	}
}

void Nvds_Init(const NvdsItem* nvdsItemArray, int count)
{
	memset(&g_Nvds, 0, sizeof(Nvds));
	g_Nvds.nvdsItemArray = nvdsItemArray;
	g_Nvds.nvdsItemArrayCount = count;

	const NvdsItem* p = nvdsItemArray;
	for (int i = 0; i < count; i++)
	{
		Nvds_InitItem(p);
	}
}