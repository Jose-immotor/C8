/*
 * File      : nvds.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * SPI FLASH
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-06     lane      first implementation
 */
 
#include "rtthread.h"
#include "common.h"
#include "nvds.h"
#include "drv_spi.h"
#include "SectorMgr.h"
#include "mx25_cmd.h"
//#include "DataRom.h"
#include "smart_system.h"
////#include "Lock.h"
//#include "Beep.h"
#include "nvc.h"
//#include<string.h>
#include "log.h"
//#include "record.h"


const NvdsMap* g_pNvdsMap = (NvdsMap*)0;	//Flash空间映射

static SectorMgr	g_MiscSectorMgr;
static SectorMgr	g_SettingSectorMgr;
static SectorMgr	g_SysCfgSectorMgr;			//调试数据数据扇区

//extern void Ble_SetSpp(Bool isEnable);

SysCfg	g_SysCfg = {0};

////检验轨迹和时间信息，是否需要保存

void NvdsMap_Dump(void)
{
	#define PRINTF_MISC(_field) Printf("\t%s=%d\n", #_field, g_MiscSectorMgr._field);
	#define PRINTF_SETTING(_field) Printf("\t%s=%d\n", #_field, g_SettingSectorMgr._field);
	#define PRINTF_CFG(_field) Printf("\t%s=%d\n", #_field, g_SysCfgSectorMgr._field);
	#define PRINTF_LOG(_field) Printf("\t%s=%d\n", #_field, g_LogRecord._field);
	#define PRINTF_LOGRECORD(_field) Printf("\t%s=%d\n", #_field, g_LogRecord.recordMgr._field);
	#define PRINTF_SYSCFG(_field) Printf("\t%s=%d\n", #_field, g_SysCfg._field);
	
	Printf("Dump NvdsMap:\n");
	
	Printf("g_MiscSectorMgr :\n");
	PRINTF_MISC(m_startAddr);
	PRINTF_MISC(m_writeOffset);//扇区内的写偏移地址
	PRINTF_MISC(m_readOffset);//扇区内的读偏移地址
	PRINTF_MISC(m_itemSize);//扇区Item大小
	PRINTF_MISC(m_ItemCount);//Item数量
	
	Printf("g_SettingSectorMgr :\n");
	PRINTF_SETTING(m_startAddr);
	PRINTF_SETTING(m_writeOffset);//扇区内的写偏移地址
	PRINTF_SETTING(m_readOffset);//扇区内的读偏移地址
	PRINTF_SETTING(m_itemSize);//扇区Item大小
	PRINTF_SETTING(m_ItemCount);//Item数量
	
	Printf("g_SysCfgSectorMgr :\n");
	PRINTF_CFG(m_startAddr);
	PRINTF_CFG(m_writeOffset);//扇区内的写偏移地址
	PRINTF_CFG(m_readOffset);//扇区内的读偏移地址
	PRINTF_CFG(m_itemSize);//扇区Item大小
	PRINTF_CFG(m_ItemCount);//Item数量
		Printf("g_SysCfg :\n");
	PRINTF_SYSCFG(readLogStartSector);//读起始扇区偏移
//	PRINTF_SYSCFG(readStartSectorInd);//
	PRINTF_SYSCFG(postLogInd);//上传Log的编号
	PRINTF_SYSCFG(readLogCount);//读取记录数

	Printf("g_LogRecord :\n");
	PRINTF_LOG(startAddr);//扇区的起始地址
	PRINTF_LOG(sectorCount);//总扇区数
	PRINTF_LOG(itemSize);//扇区Item大小
	PRINTF_LOG(total);//
	PRINTF_LOG(writeSectorInd);//
	PRINTF_LOG(readStartSectorInd);//
	PRINTF_LOG(writeSectorInd);//
	PRINTF_LOG(maxItems);//
	PRINTF_LOG(itemsPerSec);//
	PRINTF_LOG(readPointer);//
		Printf("g_LogRecord.recordMgr :\n");
	PRINTF_LOGRECORD(m_startAddr);
	PRINTF_LOGRECORD(m_writeOffset);//扇区内的写偏移地址
	PRINTF_LOGRECORD(m_readOffset);//扇区内的读偏移地址
	PRINTF_LOGRECORD(m_itemSize);//扇区Item大小
	PRINTF_LOGRECORD(m_ItemCount);//Item数量
	
}
void SysCfg_Dump()
{
	#define PRINTF_NVDS(_field) Printf("\t%s=%d\n", #_field, g_Settings._field);
	
	Printf("Dump Settings:\n");
	PRINTF_NVDS(isFlashOk);
	PRINTF_NVDS(isTakeApart);
	
	PRINTF_NVDS(IsDeactive);		//是否去激活,兼容以前版本
	PRINTF_NVDS(IsSignEnable);		//登记打卡功能使能
	PRINTF_NVDS(IsForbidDischarge);	//是否禁止放电
	PRINTF_NVDS(IsAlarmMode);		//是否境界模式
	PRINTF_NVDS(IsBatVerifyEn);		//是否使能电池身份验证
	
	PRINTF_NVDS(vol);	//音量
	PRINTF_NVDS(devcfg);
	
	PRINTF_NVDS(isLock);
	PRINTF_NVDS(isLockTest);
	
	PRINTF_NVDS(pmsFwLen);
	PRINTF_NVDS(bmsFwLen);
	PRINTF_NVDS(isBleUpg);
}

//void Nvds_Set(uint8 ind, uint32 value, uint8 isWrite)
//{	
//	#define SET_NVDS(_field) g_SysCfg._field = value; Printf("%s=%d\n", #_field, g_SysCfg._field); break;
//	#define SET_MOVE_TRACE(_field) g_MoveTrack._field = value; Printf("%s=%d\n", #_field, g_MoveTrack._field);

//	switch(ind)
//	{
//		case 0 : Nvds_SysCfgSetDefault();
//				 break;
//		default: Printf("Unknown %d.", ind);
//	}

//	if(isWrite)
//	{
//		Nvds_Write_SysCfg();
//		
//		SysCfg_Dump();
//	}
//}

Bool Nvds_Write_SysCfg()
{
	//Printf("Nvds_Write_SysCfg.\n");
	
	g_SysCfg.debugLevel = g_dwDebugLevel;
	
	g_SysCfg.timeIsValid = DATATIME_VALID;
	g_SysCfg.resetTime   = DateTime_GetSeconds(Null);
	
	return SectorMgr_Write(&g_SysCfgSectorMgr);
}

//Bool Nvds_Write_Misc()
//{
//	return SectorMgr_Write(&g_MiscSectorMgr);
//}

Bool Nvds_Write_Setting()
{
	static uint8 buf[64] = {0};
	Assert(sizeof(buf) >= g_SettingSectorMgr.m_itemSize);
	if(buf[0] == 0)
	{
		memcpy(buf, g_SettingSectorMgr.m_pItem, g_SettingSectorMgr.m_itemSize);
	}
	else if(memcmp(buf, g_SettingSectorMgr.m_pItem, g_SettingSectorMgr.m_itemSize) != 0)
	{
		memcpy(buf, g_SettingSectorMgr.m_pItem, g_SettingSectorMgr.m_itemSize);
		return SectorMgr_Write(&g_SettingSectorMgr);
	}
	return False;
}

void Nvds_InitSectorMgr(SectorMgr* pSector, uint32 startAddr, void* pItem, uint16 itemSize, uint8 version)
{
	Bool bRet = True;
	static uint8 temp[MAX_ITEM_SIZE];

	Assert(itemSize < sizeof(temp));
	
	//备份默认值
	memcpy(temp,  pItem, itemSize);
	
	//从Flash读取设置值
	bRet = SectorMgr_Init(pSector, startAddr, pItem , itemSize);
	if(bRet && version != *((uint8*)pItem))	//校验版本号
	{
		//Printf("Ver is not match.\n");
		SectorMgr_Erase(pSector);
		//恢复默认值
		memcpy(pItem, temp, itemSize);
	}
	*((uint8*)pItem) = version;
}

uint8_t sys_data[100];
S_RTC_TIME_DATA_T localDt = {0};
void Nvds_InitSysCfg()
{	
	Nvds_InitSectorMgr(&g_SysCfgSectorMgr,(uint32)&g_pNvdsMap->runCfgAddr,
					&g_SysCfg, sizeof(SysCfg), SYS_CFG_VER);
	
	g_dwDebugLevel = g_SysCfg.debugLevel;
	
	if(g_SysCfg.timeIsValid == DATATIME_VALID)
	{
		DateTime_SecondsToRtc(g_SysCfg.resetTime, &localDt);
		LocalTimeSync(&localDt);
		//DateTime_dump(&localDt);
	}
#ifdef CFG_LOG
	else
	{
		g_SysCfg.postLogInd = 0;
		g_SysCfg.readLogStartSector = g_LogRecord.readStartSectorInd;
		g_SysCfg.readLogCount = 0;
	}
	//此时还没有初始化串口，不能打印
	//如果需要打印的话，可使用rt_kprintf函数
//	PFL(DL_LOG, "postLogInd=%d,%d,%d,%d\n", g_SysCfg.readLogStartSector, g_LogRecord.readStartSectorInd, g_SysCfg.postLogInd, g_SysCfg.readLogCount);
#endif
}

void Nvds_SettingDefault()
{
	g_Settings.isLockTest = False;
	
	g_Settings.isTakeApart = False;
	g_Settings.isLock 	  = True;
	
	g_Settings.devcfg = 0;
	g_Settings.IsDeactive = True;
	g_Settings.IsSignEnable = False;
	g_Settings.IsForbidDischarge= False;
	g_Settings.IsAlarmMode	= False;
//	g_Settings.loginMaxMinute 	= SIGN_DEFAULT_MINUTE;
//	g_Settings.remainSeconds 	= g_Settings.loginMaxMinute * 60;
	g_Settings.IsBatVerifyEn 	= True;
	
#ifdef CFG_NVC
	//配置默认值
	g_Settings.LoSocPlayFlag[0] = 10;
	g_Settings.LoSocPlayFlag[1] = 20;
	g_Settings.LoSocPlayFlag[2] = 0;
	g_Settings.SocPlayFlag = BIT_2 | BIT_4;
	g_Settings.vol = VOL_DEFAULT;
	g_Settings.isVolValid = True;
#endif	
#ifdef CFG_REMOTE_ACC
	g_Settings.isRemoteAccOn = False;
#endif
}

//清除所有保存内容，恢复出厂值
void Nvds_Reset()
{
	PFL(DL_MAIN, "%s()\n", _FUNC_);
	g_SysCfg.debugLevel	= 0;	
	g_dwDebugLevel = 0;
	
	SectorMgr_Erase(&g_MiscSectorMgr);
	SectorMgr_Erase(&g_SettingSectorMgr);
	SectorMgr_Erase(&g_SysCfgSectorMgr);
	SectorMgr_Erase(&g_SysCfgSectorMgr);
	memset(&g_SysCfg, 0, sizeof(SysCfg));
#ifdef CFG_LOG	
	Log_RemoveAll();	
#endif			
	Nvds_Start();
	Nvds_SettingDefault();
	SectorMgr_Write(&g_SettingSectorMgr);

	NVC_PLAY(NVC_INFO);
}

//FLASH的SPI驱动，系统配置好
void Nvds_HwInit()
{
	spi_flash_init();
//	flash_id = spi_flash_read_id();
//	Printf("\n\rThe Flash_ID:0x%X\n\r",flash_id);
}

void Nvds_Start()
{
//	Printf("Flash used %dK, free %dk.\n", sizeof(NvdsMap) / 1024, (FlashSize - sizeof(NvdsMap)) / 1024);
	Nvds_InitSectorMgr(&g_MiscSectorMgr,(uint32)&g_pNvdsMap->miscAddr,
					&g_Settings.miscVer,&g_Settings.miscEnd - &g_Settings.miscVer, MISC_VER);
	Nvds_InitSectorMgr(&g_SettingSectorMgr,(uint32)&g_pNvdsMap->runSettingAddr,
			&g_Settings.settingVer,&g_Settings.settingEnd-&g_Settings.settingVer,SETTING_VER);	
	Nvds_Write_Setting();
	Log_Init();
	Nvds_InitSysCfg();

	g_Settings.isFlashOk = Mx25_ReadIdTest();
//	PFL(DL_MAIN, "Lock:%d\n", g_Settings.isLock);
	
	if(strlen(g_Settings.sID) == SID_LEN)
	{
//		convertMac(g_Settings.sID, g_Settings.mac);
//		g_pBle->macValid = True;
		//PFL(DL_MAIN, "BLE Mac:%s\n", g_Settings.sID);
	}
}

//void Nvds_Tester()
//{
//	Printf("Log_Add 1 test ");

//	int i = 0;
//	int* pInt = (int*)g_CommonBuf;
//	SysCfg* pSysCfg = &g_SysCfg;
//	SysCfg compare;

//	SectorMgr_Erase(&g_SysCfgSectorMgr);

//	for(i = 0; i < SECTOR_SIZE/sizeof(g_SysCfg) + 20; i++)
//	{
//		for(int j = 0; j < sizeof(g_SysCfg); j++)
//		{
//			pInt[j] = rand();
//		}
//		memcpy(pSysCfg, pInt, sizeof(g_SysCfg));
//		memcpy(&compare, pInt, sizeof(g_SysCfg));
//		g_SysCfg.version = SYS_CFG_VER;
//		pSysCfg->version = SYS_CFG_VER;
//		compare.version = SYS_CFG_VER;
//		SectorMgr_Write(&g_SysCfgSectorMgr);

//		memset(pSysCfg, 0, sizeof(g_SysCfg));
//		SectorMgr_Read(&g_SysCfgSectorMgr, pSysCfg);

//		Printf("Read=%d: ", g_SysCfgSectorMgr.m_readOffset);
//		DUMP_BYTE(pSysCfg, sizeof(g_SysCfg));
//		
//		if(memcmp(pSysCfg, &compare, sizeof(g_SysCfg) )!= 0)
//		{
//			DUMP_BYTE(pSysCfg, sizeof(g_SysCfg));
//			DUMP_BYTE(&compare, sizeof(g_SysCfg));
//			Assert(False);
//		}
//	}
//	Printf("Log_Init test passed\n");
////	Assert(False);
//}
int env_nvds_init(void)
{ 
	Smart_SettingsInit();
	DaraRom_Init();		
	Nvds_HwInit();
	LocalTimeInit();
	Nvds_Start();
	return RT_EOK;
}
INIT_ENV_EXPORT(env_nvds_init);
