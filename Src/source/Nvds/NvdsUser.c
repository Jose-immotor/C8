
#include "Common.h"
#include "NvdsUser.h"
#include "drv_spi.h"

CfgInfo	g_cfgInfo;
PdoInfo	g_pdoInfo;
DbgInfo	g_degInfo;

//g_pdoInfo�ľ���������ں�ԭֵ���Ƚϣ������Ƿ�д��FLASH
static PdoInfo g_pdoInfo_mirror;
static PdoInfo g_cfgInfo_mirror;
static PdoInfo g_degInfo_mirror;

//�������洢�����Ĺ��õĽ���������������ȡ���ֵ
static uint8_t		g_exchBuf[sizeof(PdoInfo)];
static SectorMgr    g_nvdsSecMgr[NVDS_ITEM_COUNT];

void g_pdoInfo_Dump(void)
{
	#define PRINTF_NVDS(_field) Printf("\t%s=%d\n", #_field, g_pdoInfo._field);
	
	Printf("Dump g_pdoInfo:\n");
	
	PRINTF_NVDS(isFlashOk);
	PRINTF_NVDS(isGyroOk);
}


//��Flsah��ȡ����
Bool Nvde_Read(uint32 addr, void* buf, int len)
{
	spi_flash_buffer_read(addr,buf,len);
	return True;
}

//д���ݵ�Flsah
Bool Nvds_Write(uint32 addr, const void* pData, int len)
{
	spi_flash_buffer_write(addr,(uint8_t*)pData,len);
	return True;
}

//Flsah����
void Nvds_Erase(uint32 addr, int len)
{
	spi_flash_sector_erase(addr);
}

Bool Nvds_Verify(const LogItem* pItem)
{
	return True;
}

static Bool CfgInfo_Event(CfgInfo* p, NvdsEventID eventId)
{
	//��Flash��ȡ�����ݴ����趨Ĭ��ֵ
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(CfgInfo));
	}

	return True;
}

static Bool PdoInfo_Event(PdoInfo* p, NvdsEventID eventId)
{
	//��Flash��ȡ�����ݴ����趨Ĭ��ֵ
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(PdoInfo));
	}
	return True;
}

static Bool DbgInfo_Event(DbgInfo* p, NvdsEventID eventId)
{
	//��Flash��ȡ�����ݴ����趨Ĭ��ֵ
	if (eventId == BE_DATA_ERROR)
	{
		//���ݴ����趨Ĭ��ֵ
		memset(p, 0, sizeof(DbgInfo));

		//�����������ӡ��������UARTû��ʼ�����ᵼ�³������
		g_dwDebugLevel = 0xF;
	}
	else if (eventId == BE_DATA_OK)
	{
		g_dwDebugLevel = p->debugLevel;
	}
	else if (eventId == BE_ON_WRITE_BEFORE)
	{
		p->debugLevel = g_dwDebugLevel;
	}

	return True;
}

//����ÿ���洢�ṹ�����FLASH��λ��EX_FLASH_SECTOR_SIZE
const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	{NVDS_CFG_INFO, {EX_FLASH_CFG_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_cfgInfo , sizeof(g_cfgInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase}, 
		&g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event},
	{NVDS_PDO_INFO, {EX_FLASH_CFG_PDO_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_pdoInfo , sizeof(g_pdoInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event},
	{NVDS_DBG_INFO, {EX_FLASH_CFG_DBG_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_degInfo , sizeof(g_degInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event},
};

//д��ָ����NvdsID
void NvdsUser_Write(NvdsID id)
{
	Nvds_write(id);
}

//Nvds��ʼ��������ʹ�ø�ģ����κι���֮ǰ�������ȵ��øú���
void NvdsUser_Init()
{
	uint32_t flash_id = 0;
	
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
	flash_id = spi_flash_read_id();
//	Printf("The Flash_ID:0x%X\n\r",flash_id);
	g_pdoInfo.isFlashOk = (flash_id == 0xC22015)? 1:0;
}
