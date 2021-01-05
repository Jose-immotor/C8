/*
 * File      : smart_shell.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * 和内核shell.c冲突，所以命名smart_shell.c
 * Dump 7 :打印电池信息；Dump 9：打印时间信息；Dump 10：打印g_degInfo的Sector信息
 * Dump 11：打印log的Sector信息；Dump 20：打印电池信息；Dump 21：打印NFC信息
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
 
#include "common.h"
#include "smart_shell.h"
#include "Shell_CmdLine.h"
#include "drv_onchipflash.h" 
#include "drv_usart.h"

/*!
 * \brief 打印状态信息
 *		  
 * \param  argc:打印标志
 *
 * \return NONE 
 */
void Dump(uint8_t ind)
{
	extern void BatteryInfoDump(void);
	extern void DateTime_dump(S_RTC_TIME_DATA_T* dt);
	extern void BatteryDump(void);
	extern void NfcCardReaderDump(void);
	extern void PmsDump();
	extern void g_cgfInfo_Dump(void);
	extern void g_pdoInfo_Dump(void);
	extern void g_degInfo_Dump(void);
	extern void BatteryDump2(void);
#ifdef CANBUS_MODE_JT808_ENABLE		
	extern void Sim_Dump(void);
	extern void Gprs_Dump(void);
	extern void Ble_Dump(void);
#endif 	
	extern void Cabin_Dump(void);
	extern void Adc_Dump();
	extern uint32 g_ActiveFlag;
	extern const HwFwVer AppInfo;
	
	if(1 == ind || 0 == ind) HwFwVer_Dump(Null,&AppInfo,Null);
	if(2 == ind || 0 == ind) PmsDump();
#ifdef CANBUS_MODE_JT808_ENABLE		
	if(3 == ind || 0 == ind) Sim_Dump();
	if(4 == ind || 0 == ind) Gprs_Dump();
#endif //	
	if(5 == ind) 			 Cabin_Dump();
	if(6 == ind) 			 Adc_Dump();
	if(7 == ind || 0 == ind) BatteryInfoDump();
#ifdef CANBUS_MODE_JT808_ENABLE		
	if(8 == ind || 0 == ind) Ble_Dump();
#endif //	
	if(9 == ind || 0 == ind) DateTime_dump(Null);
	if(10 == ind) 			 SectorMgr_Dump(g_NvdsItems[0].sectorMgr);
	if(11 == ind) 			 SectorMgr_Dump(g_NvdsItems[1].sectorMgr);
	if(12 == ind) 			 SectorMgr_Dump(g_NvdsItems[2].sectorMgr);
	if(13 == ind)			 SectorMgr_Dump(&g_plogMgr->record.sector);
	if(14 == ind)			 LogUser_Dump();
	
	if(20 == ind)			 g_cgfInfo_Dump();
	if(21 == ind)			 g_pdoInfo_Dump();
	if(22 == ind)			 g_degInfo_Dump();
	
	if(30 == ind)			 BatteryDump();
	if(31 == ind)			 NfcCardReaderDump();
	if(32 == ind)
	{
		uint8_t flash_read_buff[6];
		gd32_flash_read(FLASH_ADDR_UPDATE,flash_read_buff,6);
		Printf("update flag=%s.\n", flash_read_buff);

	}
	if(40 == ind)			BatteryDump2();
	//if(41 == ind)			Printf("SN=%s\n", g_cfgInfo.SN );
	//if(42 == ind )			Printf("Bat_Insert IO:%d\n",IO_Read(IO_BAT_INSERT));
	
	Printf("g_ActiveFlag=0x%x.\n", g_ActiveFlag);
	Printf("g_dwDebugLevel = 0x%08x.\n", g_dwDebugLevel);
	Printf("system_ms_tick = %d.\n", GET_TICKS());
}



void Rs485Test(uint8 value)
{
	uint8 i;
	DrvIo* g_Rs485DirCtrlIO = Null;
	
	g_Rs485DirCtrlIO = IO_Get(IO_DIR485_CTRL);
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, True);
	for(i=0;i<value;i++)
	{
		uart4_put_byte(0x5A);
	}
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, False);
}

/*!
 * \brief 设置
 *		  
 * \param  
 *
 * \return NONE 
 */
void Set(uint8 ind, uint32 value)
{

	void LocalTimeReset(void);

	extern void Nvc_SetVol(uint8 vol);
	extern void SetAccOn(uint8 on);
	extern void Cabin_UnLock();
	extern void LogUser_Reset();
	extern void CgfInfo_Reset();
	extern void PdoInfo_Reset();
	extern void DbgInfo_Reset();
	extern void endless_loop_for_wdTest();
	extern void CtrlBLE( uint16_t ctrl );
	//extern void CAN1_TxRx( uint8_t io );
	switch(ind)
	{
		case 0: LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, value);break;
		case 1: if(value>0&&value<4) NvdsUser_Write(value);break;
		case 2: LocalTimeReset(); break;
		case 3: workmode_switchStatus(WM_SLEEP); break;
		case 4: SetAccOn(value); NvdsUser_Write(NVDS_PDO_INFO);break;
		case 5: Rs485Test(value); break;
		case 6: LogUser_Reset(); break;
		case 8: Cabin_UnLock();break;
		case 9: g_cfgInfo.isActive= value;NvdsUser_Write(NVDS_CFG_INFO);break;
		case 10: g_pdoInfo.isCanbinLock =value;break;
		case 11: Nvc_SetVol(value);NvdsUser_Write(NVDS_CFG_INFO);break;
		case 20: CgfInfo_Reset(); break;
		case 21: PdoInfo_Reset(); break;
		case 22: DbgInfo_Reset(); break;
		//case 23: 
		//{
		//	uint8_t buff[6];
		//	memcpy(&buff[0], "update", 6);
		//	gd32_flash_write(FLASH_ADDR_UPDATE,buff,6);
		//	break;
		//}
		//case 24:
		//{		
		//	gd32_flash_erase(FLASH_ADDR_UPDATE,6); break;
		//}
		case 30: endless_loop_for_wdTest(); break;
		//case 32: CAN1_TxRx(value) ;break;
		case 33 :CtrlBLE( value == 1 ? 1 : 0 ); break ;
	}
}

void NvcPlay(uint8 audioInd, uint8 maxRepeat, uint8 vol)
{
	extern void Nvc_PlayEx(uint8 audioInd, uint8 maxRepeat, uint8 vol);
	
	Nvc_PlayEx(audioInd,maxRepeat,vol);
}

/*!
 * \brief 自检，检测电路板硬件
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
void SelfTest(uint8 flag)
{

	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);
	
	Printf("\r\nsID: %08X%08X%08X\r\n",sn2,sn1,sn0);
	Printf("\tFlash:%s\n"		, g_pdoInfo.isFlashOk ? "pass" : "fail");
	Printf("\tGyroscope:%s\n"	, g_pdoInfo.isGyroOk  ? "pass" : "fail");
	Printf("\tRS485Comm:%s\n"	, g_pdoInfo.isRs485Ok  ? "pass" : "fail");
	Printf("\tCANComm:%s\n"		, g_pdoInfo.isCANOk  ? "pass" : "fail");

}

/*!
 * \brief 复位
 *		  
 * \param  NONE 
 *
 * \return NONE 
 */
void SystemReset(void)
{
    Boot();
}

void Debug_SetL(uint32_t value)
{
	g_dwDebugLevel = value;
	Dbg_SetLevel(value);
	NvdsUser_Write(NVDS_DBG_INFO);
}


void Debug_SetB(uint32_t nIndex, Bool isEnable)
{
	Dbg_SetBit(nIndex, isEnable);
	NvdsUser_Write(NVDS_DBG_INFO);
}

void Log_DumpCount(int count)
{
	int ind = g_plogMgr->record.itemCount - count;
	if(ind < 0) ind = 0;

	Log_DumpByInd(g_plogMgr,ind, count);
}

void Log_DumpInd(int ind, int count)
{
	Log_DumpByInd(g_plogMgr,ind, count);
}


void Set_SN(const char *pSN)
{
	//uint8 len = strlen(pSN);
	//memset( g_cfgInfo.SN , 0 , 32 );
	//memcpy( g_cfgInfo.SN , pSN , len > 12 ? 12 : len );
	//NvdsUser_Write(NVDS_CFG_INFO);
	//Printf("Write SN: %s\n",g_cfgInfo.SN );
}

extern char rt_hw_console_getchar(void);
void Smart_Shell_run()
{
	char data;
	data = rt_hw_console_getchar();
	
	if(data != 0xff)
	{
		Shell_CmdLine_AddStrEx(&data, 1);
	}
}

void Smart_shell_init()
{
//	static CmdLineCfg cmdLineCfg;
	static Obj obj = { "SmartShell", Null, Null, Smart_Shell_run };
	ObjList_add(&obj);

	static SFnDef g_CmdTable[] =
	{
		   {(void*)Dump		  , "Dump(uint8_t ind)", 1}
		  ,{(void*)SystemReset, "Reset()", 0}
		  ,{(void*)Debug_SetL, "Debug_SetL(uint32_t value)", 1}
		  ,{(void*)Debug_SetB , "Debug_SetB(uint32_t nIndex, Bool isEnable)", 2}
		  ,{(void*)Set		  , "Set(uint8 ind, uint32 value)",2}
		  ,{(void*)SelfTest		, "SelfTest(uint8 param)"}
		  ,{(void*)NvcPlay      	, "NvcPlay(uint8 audioInd, uint8 maxRepeat, uint8 vol)"}
		  ,{(void*)Log_DumpCount 	,"Log_DumpCount(int count)"}
		  ,{(void*)Log_DumpInd  	,"Log_DumpInd(int ind, int count)"}
		  ,{(void*)Set_SN			,"Set_SN(sn)",1}
	};
	SCmdLine_Init(g_CmdTable,sizeof(g_CmdTable)/sizeof(SFnDef), TRUE ,(OutPutFun)printf);
}
