
#include "Common.h"
#include "NvdsUser.h"
#include "drv_spi.h"

CfgInfo	g_cfgInfo;
PdoInfo	g_pdoInfo;
DbgInfo	g_degInfo;

//g_pdoInfo的镜像对象，用于和原值作比较，决定是否写入FLASH
static CfgInfo g_cfgInfo_mirror;
static PdoInfo g_pdoInfo_mirror;
static DbgInfo g_dbgInfo_mirror;

//上面多个存储变量的公用的交换缓冲区，长度取最大值
static uint8_t		g_exchBuf[sizeof(CfgInfo)];
static SectorMgr    g_nvdsSecMgr[NVDS_ITEM_COUNT];

void SetAccOn(uint8 on)
{
	if(on)
	{
		g_pdoInfo.isRemoteAccOn = 1;
		Pms_postMsg(PmsMsg_accOn, 0, 0);
		LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOn,1);
	}
	else
	{
		g_pdoInfo.isRemoteAccOn = 0;
		Pms_postMsg(PmsMsg_accOff, 0, 0);
		LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOff,1);
	}
}

void g_cgfInfo_Dump(void)
{
	#define PRINTF_CFG(_field) Printf("\t%s=%d\n", #_field, g_cfgInfo._field);
	
	Printf("Dump g_cfgInfo:\n");
	PRINTF_CFG(isActive);
	PRINTF_CFG(vol);
//	PRINTF_CFG(isAccOn);

}
/*
uint8 vol;					//音量
		uint8 isAccOn:1;			//远程控制ACC  ！！取消，用下面的
		uint8 reserved0:7;
		uint8 Reserved[14];	//保留15个字节
		//uint8 SN[32];		// SN Jose 添加
		uint8 BeaconUUID[16];

*/
void g_pdoOkInfo_Dump(void)
{
	#define PRINTF_PDO(_field) Printf("\t%s=%d\n", #_field, g_pdoInfo._field);
		
	PRINTF_PDO(isFlashOk);
	PRINTF_PDO(isGyroOk);
	PRINTF_PDO(isRs485Ok);
	PRINTF_PDO(isCANOk);
}

void g_pdoInfo_Dump(void)
{
	#define PRINTF_PDO(_field) Printf("\t%s=%d\n", #_field, g_pdoInfo._field);
	
	Printf("Dump g_pdoInfo:\n");
	
	PRINTF_PDO(isRemoteAccOn);//车辆远程点火
	PRINTF_PDO(isWheelLock);///轮毂锁
	PRINTF_PDO(isCanbinLock);///座舱锁
	PRINTF_PDO(isLowPow);
	
	
	PRINTF_PDO(isFlashOk);
	PRINTF_PDO(isGyroOk);
	PRINTF_PDO(isNfcOk);
	PRINTF_PDO(isTakeApart);
	PRINTF_PDO(timeStamp);
}

void g_degInfo_Dump(void)
{
	#define PRINTF_DEG(_field) Printf("\t%s=%8x\n", #_field, g_degInfo._field);
	
	Printf("Dump g_degInfo:\n");
	
	PRINTF_DEG(debugLevel);
}

//从Flsah读取数据
Bool Nvde_Read(uint32 addr, void* buf, int len)
{
	spi_flash_buffer_read(addr,buf,len);
	return True;
}

//写数据到Flsah
Bool Nvds_Write(uint32 addr, const void* pData, int len)
{
	spi_flash_buffer_write(addr,(uint8_t*)pData,len);
	return True;
}

//Flsah擦除
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
	else if (eventId == BE_ON_WRITE_BEFORE)
	{
		p->timeStamp = DateTime_GetSeconds(Null);
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
		//默认只打开主要的调试开关
		//g_dwDebugLevel = 0xF | DL_NFC;
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

//配置每个存储结构存放在FLASH的位置EX_FLASH_SECTOR_SIZE
const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	{NVDS_CFG_INFO, {EX_FLASH_CFG_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_cfgInfo , sizeof(g_cfgInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase}, 
		&g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event, & g_cfgInfo_mirror},
	{NVDS_PDO_INFO, {EX_FLASH_CFG_PDO_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_pdoInfo , sizeof(g_pdoInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event,& g_pdoInfo_mirror},
	{NVDS_DBG_INFO, {EX_FLASH_CFG_DBG_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_degInfo , sizeof(g_degInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event,& g_dbgInfo_mirror},
};

//清除所有保存内容，恢复出厂值
void NvdsUser_Reset()
{
	SectorMgr_Erase(g_NvdsItems[0].sectorMgr);
	SectorMgr_Erase(g_NvdsItems[1].sectorMgr);
	SectorMgr_Erase(g_NvdsItems[2].sectorMgr);
}
void CgfInfo_Reset()
{
	SectorMgr_Erase(g_NvdsItems[0].sectorMgr);
}
void PdoInfo_Reset()
{
	SectorMgr_Erase(g_NvdsItems[1].sectorMgr);
}
void DbgInfo_Reset()
{
	SectorMgr_Erase(g_NvdsItems[2].sectorMgr);
}

//写入指定的NvdsID
void NvdsUser_Write(NvdsID id)
{
	Nvds_write(id);
}

//Nvds初始化函数，使用改模块的任何功能之前，必须先调用该函数
void NvdsUser_Init()
{
	uint32_t flash_id = 0;
	
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
	flash_id = spi_flash_read_id();
	Printf("The Flash_ID:0x%X\n\r",flash_id);
	g_pdoInfo.isFlashOk = (flash_id == 0xC22015)? 1:0;
	if(g_pdoInfo.timeStamp)
	{
		S_RTC_TIME_DATA_T localDt = {0};
		DateTime_SecondsToRtc(g_pdoInfo.timeStamp, &localDt);
		LocalTimeSync(&localDt);
		//DateTime_dump(&localDt);
	}
	if(!g_cfgInfo.vol)
	{
		g_cfgInfo.vol= 7 ;//默认音量为7
	}
//	if(!g_pdoInfo.isRemoteAccOn)
//	{
//		g_pdoInfo.isRemoteAccOn=1;
//	}
}
