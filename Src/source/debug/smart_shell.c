/*
 * File      : smart_shell.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * ���ں�shell.c��ͻ����������smart_shell.c
 * Dump 7 :��ӡ�����Ϣ��Dump 9����ӡʱ����Ϣ��Dump 10����ӡg_degInfo��Sector��Ϣ
 * Dump 11����ӡlog��Sector��Ϣ��Dump 20����ӡ�����Ϣ��Dump 21����ӡNFC��Ϣ
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
 
#include "common.h"

/*!
 * \brief ��ӡ״̬��Ϣ
 *		  
 * \param  argc:��ӡ��־
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
	extern void PmsDump();
	extern void g_cgfInfo_Dump(void);
	extern void g_pdoInfo_Dump(void);
	extern void g_degInfo_Dump(void);

	extern void Adc_Dump();
	extern uint32 g_ActiveFlag;
	extern const HwFwVer AppInfo;
	
	sscanf(&(*argv[1]), "%d", &ind);
	if(1 == ind || 0 == ind) HwFwVer_Dump(Null,&AppInfo,Null);
	if(2 == ind || 0 == ind) PmsDump();
////	if(3 == ind || 0 == ind) Gps_Dump();
////	if(4 == ind || 0 == ind) Gprs_Dump();
	
	if(6 == ind) 			 Adc_Dump();
	if(7 == ind || 0 == ind) BatteryInfoDump();
	if(9 == ind || 0 == ind) DateTime_dump(Null);
	if(10 == ind) 			 SectorMgr_Dump(g_NvdsItems[0].sectorMgr);
	if(11 == ind) 			 SectorMgr_Dump(g_NvdsItems[1].sectorMgr);
	if(12 == ind) 			 SectorMgr_Dump(g_NvdsItems[2].sectorMgr);
	if(13 == ind)			 SectorMgr_Dump(&g_plogMgr->record.sector);
	
	if(20 == ind)			 g_cgfInfo_Dump();
	if(21 == ind)			 g_pdoInfo_Dump();
	if(22 == ind)			 g_degInfo_Dump();
	
	if(30 == ind)			 BatteryDump();
	if(31 == ind)			 NfcCardReaderDump();
	
	Printf("g_ActiveFlag=0x%x\n", g_ActiveFlag);
	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);
	Printf("system_ms_tick = %d\n", GET_TICKS());
}
MSH_CMD_EXPORT(Dump, Dump sample: Dump <uint8_t ind>);

extern uint32_t uart4_put_byte(uint8_t data);
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
 * \brief ����
 *		  
 * \param  
 *
 * \return NONE 
 */
static void Set(int argc, char**argv)
{
	int ind = 0;
	uint32 value;
	void LocalTimeReset(void);

	extern void Nvc_SetVol(uint8 vol);
	extern void SetAccOn(uint8 on);
	
	sscanf(&(*argv[1]), "%d", &ind);
	sscanf(&(*argv[2]), "%d", &value);
	switch(ind)
	{
		case 0: LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, value);break;
		case 1: if(value>0&&value<4) NvdsUser_Write(value);break;
		case 2: LocalTimeReset(); break;
		case 3: if(value>0&&value<5) Pms_switchStatus(value); break;
		case 4: SetAccOn(value); break;
		case 5: Rs485Test(value); break;
////		case 6: g_Settings.IsBatVerifyEn=value; Sign_Dump(Null); Nvds_Write_Setting(); break;
////		case 7: g_Settings.IsAlarmMode=value; Sign_Dump(Null); break;
////		case 8: SET_VAR(g_ForcePmsDischarge); 
////		case 9: SET_VAR(g_ForceBatSoc); 
////		case 10: Sim_PowerReset(0); break;
		case 11: Nvc_SetVol(value); break;
////		case 12: Fsm_StateKeyOff(MSG_FORCE_POWERDOWN); break;
//		case 13: Nvc_PlayEx(value); break;
	}
//		
	//Nvds_Write_Setting();
}
MSH_CMD_EXPORT(Set, Set sample: Set <uint8_t ind uint32 value>);

static void NvcPlay(int argc, char**argv)
{
	int audioInd = 0;
	int maxRepeat = 0;
	int vol = 0;

	extern void Nvc_PlayEx(uint8 audioInd, uint8 maxRepeat, uint8 vol);
	
	sscanf(&(*argv[1]), "%d", &audioInd);
	sscanf(&(*argv[2]), "%d", &maxRepeat);
	sscanf(&(*argv[3]), "%d", &vol);
	Nvc_PlayEx(audioInd,maxRepeat,vol);
}
MSH_CMD_EXPORT(NvcPlay, NvcPlay sample: NvcPlay <audioInd maxRepeatvol>);

/*!
 * \brief �Լ죬����·��Ӳ��
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
static void SelfTest(void)
{
	extern void g_pdoOkInfo_Dump(void);
	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);
	
	Printf("\r\nsID: %X%X%X\r\n",sn2,sn1,sn0);
	g_pdoOkInfo_Dump();	
}
MSH_CMD_EXPORT(SelfTest , SelfTest board);

/*!
 * \brief ��λ
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

