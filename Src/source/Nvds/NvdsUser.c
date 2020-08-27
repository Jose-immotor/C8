
#include "Common.h"
#include "NvdsUser.h"

#define NVDS_ITEM_COUNT 3

static CfgInfo	g_cfgInfo;
static PdoInfo	g_pdoInfo;
static DbgInfo	g_degInfo;

//�������洢�����Ĺ��õĽ���������������ȡ���ֵ
static uint8_t		g_exchBuf[sizeof(PdoInfo)];
static SectorMgr    g_nvdsSecMgr[NVDS_ITEM_COUNT];

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

const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	{NVDS_CFG_INFO, {CFG_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_cfgInfo , sizeof(g_cfgInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event},
	{NVDS_PDO_INFO, {PDO_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_pdoInfo , sizeof(g_pdoInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event},
	{NVDS_DBG_INFO, {DBG_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_degInfo , sizeof(g_degInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event},
};

//д��ָ����NvdsID
void NvdsUser_Write(NvdsID id)
{
	Nvds_write(id);
}

//Nvds��ʼ��������ʹ�ø�ģ����κι���֮ǰ�������ȵ��øú���
void NvdsUser_Init()
{
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
}
