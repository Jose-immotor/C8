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

/*!
 * \brief 打印状态信息
 *		  
 * \param  argc:打印标志
 *
 * \return NONE 
 */
static void Dump(int argc, char**argv)
{
	int ind = 0;
	extern void BatteryInfoDump(void);
	extern void DateTime_dump(S_RTC_TIME_DATA_T* dt);
	extern void BatteryDump(void);
	extern void NfcCardReaderDump(void);
//	extern uint32 g_ActiveFlag;

	sscanf(&(*argv[1]), "%d", &ind);
//	if(1 == ind || 0 == ind) DaraRom_Dump();
////	if(2 == ind || 0 == ind) Sign_Dump();
////	if(3 == ind || 0 == ind) Gps_Dump();
////	if(4 == ind || 0 == ind) Gprs_Dump();
////	if(5 == ind || 0 == ind) AtCmdCtrl_Dump(g_pSimAtCmdCtrl);
////	if(6 == ind || 0 == ind) AtCmdCtrl_Dump(g_pBleAtCmdCtrl);
	if(7 == ind || 0 == ind) BatteryInfoDump();
	if(9 == ind || 0 == ind) DateTime_dump(Null);
	if(10 == ind) 			 SectorMgr_Dump(g_NvdsItems[2].sectorMgr);
	if(11 == ind)			 SectorMgr_Dump(&g_plogMgr->record.sector);
	
	if(20 == ind)			 BatteryDump();
	if(21 == ind)			 NfcCardReaderDump();
	
//	Printf("g_ActiveFlag=0x%x\n", g_ActiveFlag);
	Printf("g_dwDebugLevel = 0x%04x\n", g_dwDebugLevel);
}
MSH_CMD_EXPORT(Dump, Dump sample: Dump <uint8_t ind>);

//void SetFwVer(uint8_t mainVer, uint8_t SubVer, uint8_t MinorVer, uint32 buildeNum)
//{
//	g_pDataRom->m_AppMainVer   = mainVer;
//	g_pDataRom->m_AppSubVer    = SubVer;
//	g_pDataRom->m_AppMinorVer  = MinorVer;
//	g_pDataRom->m_AppBuildeNum = buildeNum;
//	DataRom_Write();
//	Printf("OK.\n");
//}

//void SetHwVer(uint8_t mainVer, uint8_t SubVer)
//{
//	g_pDataRom->m_HwMainVer   = mainVer;
//	g_pDataRom->m_HwSubVer    = SubVer;
//	
//	DataRom_Write();
//	Printf("OK.\n");
//}

/*!
 * \brief 设置
 *		  
 * \param  
 *
 * \return NONE 
 */
static void Set(int argc, char**argv)
{
	int ind = 0;
	uint32 value;

	sscanf(&(*argv[1]), "%d", &ind);
	sscanf(&(*argv[2]), "%d", &value);
	switch(ind)
	{
		case 0: LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, value);break;
////		case 1: SetActive(value); Nvds_Write_Setting(); break;
////		case 2: SetForbidDischarge(value); Nvds_Write_Setting(); break;
////		case 3: SetSignEn(value); Nvds_Write_Setting(); break;
////		case 4: Sign_SetMaxTime(value); break;
////		case 5: Sign_DisableTimerReset(value); break;
////		case 6: g_Settings.IsBatVerifyEn=value; Sign_Dump(Null); Nvds_Write_Setting(); break;
////		case 7: g_Settings.IsAlarmMode=value; Sign_Dump(Null); break;
////		case 8: SET_VAR(g_ForcePmsDischarge); 
////		case 9: SET_VAR(g_ForceBatSoc); 
////		case 10: Sim_PowerReset(0); break;
////		case 11: Nvc_SetVol(value); break;
////		case 12: Fsm_StateKeyOff(MSG_FORCE_POWERDOWN); break;
//		case 13: RTC_TimerStart(value); break;
	}
//		
	//Nvds_Write_Setting();
}
MSH_CMD_EXPORT(Set, Set sample: Set <uint8_t ind uint32 value>);

/*!
 * \brief 自检，检测电路板硬件
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
static void SelfTest(void)
{
	extern void ErrList_Dump(void);
	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);
	
	Printf("\r\nsID: %X%X%X\r\n",sn2,sn1,sn0);

//	Printf("\tGprsCSQ:[%d]\n", g_pSimCard->csq);
////	if(flag)
//	{
//		Printf("\tMoto ErrCode1:0x%x\n", GetErrorCode(ERR_TYPE_MOTOR1));
//		Printf("\tBat1 ErrCode:0x%x\n", GetErrorCode(ERR_TYPE_BATTERY1));
//		Printf("\tBat2 ErrCode:0x%x\n", GetErrorCode(ERR_TYPE_BATTERY1));
//		Printf("\tErrCode:0x%x\n", GetErrorCodeDec());
//		ErrList_Dump();
//	}
}
MSH_CMD_EXPORT(SelfTest , SelfTest board);

/*!
 * \brief 复位
 *		  
 * \param  NONE 
 *
 * \return NONE 
 */
static void Reset(void)
{
    Boot();
}
MSH_CMD_EXPORT(Reset, Reboot System);

static void Debug_SetL(int argc, char**argv)
{
	uint32_t value;

	sscanf(&(*argv[1]), "%d", &value);
	g_dwDebugLevel = value;
	Dbg_SetLevel(value);
	NvdsUser_Write(NVDS_DBG_INFO);
}
MSH_CMD_EXPORT(Debug_SetL, Debug_SetL sample: Debug_SetL <uint32_t ind>);

static void Debug_SetB (int argc, char**argv)
{
	uint32_t nIndex;
	int isEnable;

	sscanf(&(*argv[1]), "%d", &nIndex);
	sscanf(&(*argv[2]), "%d", &isEnable);
	Dbg_SetBit(nIndex, isEnable);
	NvdsUser_Write(NVDS_DBG_INFO);
}
MSH_CMD_EXPORT(Debug_SetB, Debug_SetL sample: Debug_SetL <uint32_t ind>);


//static void LogDumpCount(int argc, char**argv)
//{
//	int count;

//	sscanf(&(*argv[1]), "%d", &count);
////	Log_DumpByCount(g_plogMgr, count);
//}
//MSH_CMD_EXPORT(LogDumpCount, LogUser_DumpByCount);

static void LogDumpInd(int argc, char**argv)
{
	int ind = 0;
	int count;

	sscanf(&(*argv[1]), "%d", &ind);
	sscanf(&(*argv[2]), "%d", &count);
	Log_DumpByInd(g_plogMgr, ind, count);
}
MSH_CMD_EXPORT(LogDumpInd, LogUser_DumpByInd);
