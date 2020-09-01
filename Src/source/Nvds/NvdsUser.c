
#include "Common.h"
#include "NvdsUser.h"

#define NVDS_ITEM_COUNT 3

CfgInfo	g_cfgInfo;
PdoInfo	g_pdoInfo;
DbgInfo	g_degInfo;

//g_pdoInfo的镜像对象，用于和原值作比较，决定是否写入FLASH
static PdoInfo g_pdoInfo_mirror;
static PdoInfo g_cfgInfo_mirror;
static PdoInfo g_degInfo_mirror;

//上面多个存储变量的公用的交换缓冲区，长度取最大值
static uint8_t		g_exchBuf[sizeof(PdoInfo)];
static SectorMgr    g_nvdsSecMgr[NVDS_ITEM_COUNT];

static Bool CfgInfo_Event(CfgInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(CfgInfo));
	}

	return True;
}

static Bool PdoInfo_Event(PdoInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(PdoInfo));
	}
	return True;
}

static Bool DbgInfo_Event(DbgInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		//数据错误，设定默认值
		memset(p, 0, sizeof(DbgInfo));

		//不能再这里打印输出，如果UART没初始化，会导致程序堵塞
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

//配置每个存储结构存放在FLASH的位置
const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	{NVDS_CFG_INFO, {CFG_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_cfgInfo , sizeof(g_cfgInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event, &g_pdoInfo_mirror},
	{NVDS_PDO_INFO, {PDO_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_pdoInfo , sizeof(g_pdoInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event, &g_cfgInfo_mirror},
	{NVDS_DBG_INFO, {DBG_INFO_ADDR, MCU_FLASH_SECTOR_SIZE, &g_degInfo , sizeof(g_degInfo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event, &g_degInfo_mirror},
};

//写入指定的NvdsID
void NvdsUser_Write(NvdsID id)
{
	Nvds_write(id);
}

//Nvds初始化函数，使用改模块的任何功能之前，必须先调用该函数
void NvdsUser_Init()
{
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
}
