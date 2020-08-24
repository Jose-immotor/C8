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
#include "CcuLog.h"
//#include "Beep.h"
#include "nvc.h"
#include "main.h"
#include "LogUser.h"
#include "log.h"
#include "MemMap.h"
#include "CcuDef.h"


const NvdsMap* g_pNvdsMap = (NvdsMap*)0;	//Flash�ռ�ӳ��

static SectorMgr	g_MiscSectorMgr;
static SectorMgr	g_SettingSectorMgr;
static SectorMgr	g_SysCfgSectorMgr;			//����������������

static Nvds  g_Nvds[4];

SysCfg	g_SysCfg = {0};

DebugInfo	*g_pDbgInfo;

////����켣��ʱ����Ϣ���Ƿ���Ҫ����

void NvdsMap_Dump(void)
{
	#define PRINTF_MISC(_field) Printf("\t%s=%d\n", #_field, g_MiscSectorMgr._field);
	#define PRINTF_SETTING(_field) Printf("\t%s=%d\n", #_field, g_SettingSectorMgr._field);
	#define PRINTF_CFG(_field) Printf("\t%s=%d\n", #_field, g_SysCfgSectorMgr._field);
	#define PRINTF_LOG(_field) //Printf("\t%s=%d\n", #_field, g_LogRecord._field);
	#define PRINTF_LOGRECORD(_field) //Printf("\t%s=%d\n", #_field, g_LogRecord.recordMgr._field);
	#define PRINTF_SYSCFG(_field) Printf("\t%s=%d\n", #_field, g_SysCfg._field);
	
	Printf("Dump NvdsMap:\n");
	
	Printf("g_MiscSectorMgr :\n");
	PRINTF_MISC(m_startAddr);
	PRINTF_MISC(m_writeOffset);//�����ڵ�дƫ�Ƶ�ַ
	PRINTF_MISC(m_readOffset);//�����ڵĶ�ƫ�Ƶ�ַ
	PRINTF_MISC(m_itemSize);//����Item��С
	PRINTF_MISC(m_ItemCount);//Item����
	
	Printf("g_SettingSectorMgr :\n");
	PRINTF_SETTING(m_startAddr);
	PRINTF_SETTING(m_writeOffset);//�����ڵ�дƫ�Ƶ�ַ
	PRINTF_SETTING(m_readOffset);//�����ڵĶ�ƫ�Ƶ�ַ
	PRINTF_SETTING(m_itemSize);//����Item��С
	PRINTF_SETTING(m_ItemCount);//Item����
	
	Printf("g_SysCfgSectorMgr :\n");
	PRINTF_CFG(m_startAddr);
	PRINTF_CFG(m_writeOffset);//�����ڵ�дƫ�Ƶ�ַ
	PRINTF_CFG(m_readOffset);//�����ڵĶ�ƫ�Ƶ�ַ
	PRINTF_CFG(m_itemSize);//����Item��С
	PRINTF_CFG(m_ItemCount);//Item����
		Printf("g_SysCfg :\n");
	PRINTF_SYSCFG(readLogStartSector);//����ʼ����ƫ��
//	PRINTF_SYSCFG(readStartSectorInd);//
	PRINTF_SYSCFG(postLogInd);//�ϴ�Log�ı��
	PRINTF_SYSCFG(readLogCount);//��ȡ��¼��

	Printf("g_LogRecord :\n");
	PRINTF_LOG(startAddr);//��������ʼ��ַ
	PRINTF_LOG(sectorCount);//��������
	PRINTF_LOG(itemSize);//����Item��С
	PRINTF_LOG(itemCount);//
	PRINTF_LOG(writeSectorInd);//
	PRINTF_LOG(readStartSectorInd);//
	PRINTF_LOG(writeSectorInd);//
	PRINTF_LOG(maxItems);//
	PRINTF_LOG(itemsPerSec);//
	PRINTF_LOG(readPointer);//
		Printf("g_LogRecord.recordMgr :\n");
	PRINTF_LOGRECORD(m_startAddr);
	PRINTF_LOGRECORD(m_writeOffset);//�����ڵ�дƫ�Ƶ�ַ
	PRINTF_LOGRECORD(m_readOffset);//�����ڵĶ�ƫ�Ƶ�ַ
	PRINTF_LOGRECORD(m_itemSize);//����Item��С
	PRINTF_LOGRECORD(m_ItemCount);//Item����
	
}
void SysCfg_Dump()
{
	#define PRINTF_NVDS(_field) Printf("\t%s=%d\n", #_field, g_Settings._field);
	
	Printf("Dump Settings:\n");
	PRINTF_NVDS(isFlashOk);
	PRINTF_NVDS(isTakeApart);
	
	PRINTF_NVDS(IsDeactive);		//�Ƿ�ȥ����,������ǰ�汾
	PRINTF_NVDS(IsSignEnable);		//�ǼǴ򿨹���ʹ��
	PRINTF_NVDS(IsForbidDischarge);	//�Ƿ��ֹ�ŵ�
	PRINTF_NVDS(IsAlarmMode);		//�Ƿ񾳽�ģʽ
	PRINTF_NVDS(IsBatVerifyEn);		//�Ƿ�ʹ�ܵ�������֤
	
	PRINTF_NVDS(vol);	//����
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
	
	//����Ĭ��ֵ
	memcpy(temp,  pItem, itemSize);
	
	//��Flash��ȡ����ֵ
	bRet = SectorMgr_Init(pSector, SECTOR_SIZE, startAddr, pItem , itemSize);
	if(bRet && version != *((uint8*)pItem))	//У��汾��
	{
		//Printf("Ver is not match.\n");
		SectorMgr_Erase(pSector);
		//�ָ�Ĭ��ֵ
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
//		g_SysCfg.readLogStartSector = g_LogRecord.readStartSectorInd;
		g_SysCfg.readLogCount = 0;
	}
	//��ʱ��û�г�ʼ�����ڣ����ܴ�ӡ
	//�����Ҫ��ӡ�Ļ�����ʹ��rt_kprintf����
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
	//����Ĭ��ֵ
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

//������б������ݣ��ָ�����ֵ
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
//	Log_RemoveAll();	
#endif			
	Nvds_Start();
	Nvds_SettingDefault();
	SectorMgr_Write(&g_SettingSectorMgr);

	NVC_PLAY(NVC_INFO);
}

//FLASH��SPI������ϵͳ���ú�
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
	LogUser_Init();
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

void DbgInfo_Event(DebugInfo* p, BlockEventID eventId)
{
	if (eventId == BE_DATA_ERROR)
	{
		//���ݴ����趨Ĭ��ֵ
		memset(p, 0, sizeof(DebugInfo));

		//�����������ӡ�����UARTû��ʼ�����ᵼ�³������
		//Printf("DbgInfo error.\n");

		g_dwDebugLevel = 0xFF;
	}
	else if (eventId == BE_DATA_OK)
	{
		g_dwDebugLevel = p->debugLevel;
	}
	else if (eventId == BE_ON_WRITE_BEFORE)
	{
		p->debugLevel = g_dwDebugLevel;
	}
}

void Nvds_InitItem(Nvds* p, uint8 id, void* pData, int dataLength, void* eepRomAddr, BlockEventFn event)
{
	Assert(id < GET_ELEMENT_COUNT(g_Nvds));
	Assert(dataLength % 2 == 0);	//dataLength����Ϊż��(2�ֽڶ���)�������޷�д��Flash

	uint8* pByte = (uint8*)pData;
	p->id = id;
	p->Event = event;

	SectorMgr_Init(&p->sectorMgr, SECTOR_SIZE, (uint32)eepRomAddr, pData, dataLength);

	//���������Ƿ���Ч
	if (pByte[0] != EEPROM_FIRST_BYTE || pByte[dataLength - 1] != EEPROM_LATEST_BYTE)
	{
		//������Ч�����µ���Ĭ�ϵĳ�ʼ������
		p->Event(pByte, BE_DATA_ERROR);

		pByte[0] = EEPROM_FIRST_BYTE;
		pByte[dataLength - 1] = EEPROM_LATEST_BYTE;
		if(p->sectorMgr.m_ItemCount)	//�����д��,��ִ��ɾ������
		{
			SectorMgr_Erase(&p->sectorMgr);
		}
	}
	else
	{
		p->Event(pByte, BE_DATA_OK);
	}

}

int env_nvds_init(void)
{
	static DebugInfo	g_dbgInfo;
	
	g_pDbgInfo = &g_dbgInfo;
	
	Smart_SettingsInit();
	DaraRom_Init();
	Nvds_HwInit();
	LocalTimeInit();
	
	Nvds* p = g_Nvds;
	Nvds_InitItem(p++, NVDS_DBG_INFO, &g_dbgInfo, sizeof(g_dbgInfo), &g_pMemMap->dbgInfo, (BlockEventFn)DbgInfo_Event);
	
	LogUser_Init();
	LOG_TRACE1(CCU_ADDR, CCU_CATID_COMMON, ET_SUBID_UNDEFIND, CCU_ET_MCU_RESET, 0);
	return RT_EOK;
}
void thread_nvds_entry(void* parameter)
{
	env_nvds_init();	
    while (1)
    {
		rt_thread_mdelay(100);
    }
}

struct rt_thread thread_nvds;
unsigned char thread_nvds_stack[2048];

static int app_nvds_init(void)
{
	rt_err_t res;
	res=rt_thread_init(&thread_nvds,"nvds",thread_nvds_entry, RT_NULL,&thread_nvds_stack[0],
    sizeof(thread_nvds_stack), 5, 10);	
    if (res == RT_EOK) /* �������߳̿��ƿ飬��������߳� */
        rt_thread_startup(&thread_nvds);
	else
		rt_kprintf("\n!!create thread nvds failed!\n");
    return 0;
}
INIT_APP_EXPORT(app_nvds_init);
