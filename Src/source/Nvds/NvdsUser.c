
#include "Common.h"
#include "NvdsUser.h"

#define NVDS_ITEM_COUNT 3

static CfgInfo		g_cfgInfo;
static SysPdo		g_sysPdo;
static DebugInfo	g_debugPdo;

//上面多个存储变量的公用的交换缓冲区，长度取最大值
static uint8_t		g_exchBuf[sizeof(SysPdo)];
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

static Bool PdoInfo_Event(SysPdo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(SysPdo));
	}

	return True;
}

static Bool DbgInfo_Event(DebugInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		//数据错误，设定默认值
		memset(p, 0, sizeof(DebugInfo));

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

const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	{NVDS_CFG_INFO, {CFG_INFO_ADDR, 512, &g_cfgInfo , sizeof(g_cfgInfo) , g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event},
	{NVDS_PDO_INFO, {PDO_INFO_ADDR, 512, &g_sysPdo  , sizeof(g_sysPdo)  , g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event},
	{NVDS_DBG_INFO, {DBG_INFO_ADDR, 512, &g_debugPdo, sizeof(g_debugPdo), g_exchBuf, sizeof(g_exchBuf)}, &g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event},
};

void NvdsUser_Init()
{
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
}
