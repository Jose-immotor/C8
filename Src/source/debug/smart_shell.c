/*
 * File      : smart_shell.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * 和内核shell.c冲突，所以命名smart_shell.c
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
 
 #include <stdio.h>
//#include "ntag_app.h"
////#include "battery_process.h"
#include <stdio.h>
#include <stdint.h>
//#include "cmdline.h"
////#include "fsm.h"
////#include "AtCmdCtrl.h"
////#include "Ble.h"
////#include "sim.h"
////#include "datarom.h"
////#include "Nvds.h"
#include "Pms.h"
////#include "Gyro.h"
////#include "Beep.h"
////#include "Sign.h"
////#include "UtpDef.h"
////#include "nvc.h"
////#include "gprs.h"
#include "gd32f403.h"

//#define SHELL_RX_BUF_SIZE 64
//#define DBL_IS_TRUE(dl) ((dl & g_dwDebugLevel) > 0)
//#define SET_DBL_BIT(dl, isTrue) \
//	if(isTrue)	\
//		g_dwDebugLevel |= (1 << GetBitInd(dl));	\
//	else	\
//		g_dwDebugLevel &= ~(1 << GetBitInd(dl));

////SafeBuf g_Uart0SafeBuf;

//extern void _puts (const char *s);
//extern void Ble_Dump(void);
//	

//smart_shell_param_t smart_shell_param;

//void SimSendAtCmd(const char* pAtCmd, uint32 second)
//{
//	uint8_t buf[64] = {0};
//	uint32 cmdLen = strlen(pAtCmd);

//	memcpy(buf, pAtCmd, cmdLen);
//	
//	buf[cmdLen++] = '\r';
//	buf[cmdLen++] = '\n';
//	buf[cmdLen] = 0;

//	//Printf("%s", buf);
////	Sim_TxAtCmd((char*)buf, cmdLen);
//}

//void BleSendAtCmd(const char* pAtCmd)
//{	
//	uint8_t buf[64] = {0};
//	uint32 cmdLen = strlen(pAtCmd);

//	memcpy(buf, pAtCmd, cmdLen);
//	
//	buf[cmdLen++] = '\r';
//	buf[cmdLen++] = '\n';
//	buf[cmdLen] = 0;
//	//Printf("%s", buf);
//	
////	Ble_Send((char*)buf, cmdLen);
//}

//void FlashTest()
//{
//	extern uint8_t Mx25_ReadIdTest( void );

//	Mx25_ReadIdTest();

//}

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
//	extern void SysCfg_Dump(void);

//	extern void Sign_Dump();
//	extern void power_dump();
//	extern int g_MaxGpsCount;
//	extern uint32 g_ActiveFlag;
//	
	sscanf(&(*argv[1]), "%d", &ind);
//	if(1 == ind || 0 == ind) DaraRom_Dump();
//	if(2 == ind || 0 == ind) Sign_Dump();
//	if(3 == ind || 0 == ind) Gps_Dump();
//	if(4 == ind || 0 == ind) Gprs_Dump();
//	if(5 == ind || 0 == ind) AtCmdCtrl_Dump(g_pSimAtCmdCtrl);
//	if(6 == ind || 0 == ind) AtCmdCtrl_Dump(g_pBleAtCmdCtrl);
	if(7 == ind || 0 == ind) BatteryDump(Null);
//	if(8 == ind || 0 == ind) Ble_Dump();
//	if(9 == ind || 0 == ind) DateTime_dump(Null);
//	if(10 == ind) NvdsMap_Dump();
//	if(11 == ind || 0 == ind)SysCfg_Dump();
//	if(12 == ind || 0 == ind)power_dump();
//	if(13 == ind || 0 == ind)adc_dump();

//	Printf("g_MaxGpsCount=%d\n", g_MaxGpsCount);
//	Printf("g_ActiveFlag=0x%x\n", g_ActiveFlag);
//	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);
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

///*!
// * \brief 设置
// *		  
// * \param  
// *
// * \return NONE 
// */
//static void Set(int argc, char**argv)
//{
//	extern void Fsm_StateKeyOff(uint8 msgID);
//	extern uint8_t g_ForceBatSoc;
//	extern uint8_t g_ForcePmsDischarge;
//	#define SET_VAR(_field) _field = value; Printf("%s=%d\n", #_field, value); break
//	#define SET(_field) _field = value; Printf("%s=%d\n", #_field, value)
//	
//	int ind = 0;
//	uint32 value;
//	
//	sscanf(&(*argv[1]), "%d", &ind);
//	sscanf(&(*argv[2]), "%d", &value);
//	switch(ind)
//	{
//		case 0: SET_VAR(g_TestFlag);
//		case 1: SetActive(value); Nvds_Write_Setting(); break;
//		case 2: SetForbidDischarge(value); Nvds_Write_Setting(); break;
//		case 3: SetSignEn(value); Nvds_Write_Setting(); break;
//		case 4: Sign_SetMaxTime(value); break;
//		case 5: Sign_DisableTimerReset(value); break;
//		case 6: g_Settings.IsBatVerifyEn=value; Sign_Dump(Null); Nvds_Write_Setting(); break;
//		case 7: g_Settings.IsAlarmMode=value; Sign_Dump(Null); break;
//		case 8: SET_VAR(g_ForcePmsDischarge); 
//		case 9: SET_VAR(g_ForceBatSoc); 
//		case 10: Sim_PowerReset(0); break;
//		case 11: Nvc_SetVol(value); break;
//		case 12: Fsm_StateKeyOff(MSG_FORCE_POWERDOWN); break;
//		case 13: RTC_TimerStart(value, RTC_WAKEUP_TIMEID); break;
//	}
//		
//	//Nvds_Write_Setting();
//}
//MSH_CMD_EXPORT(Set, Set sample: Set <uint8_t ind uint32 value>);

/*!
 * \brief 自检，检测电路板硬件
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
static void SelfTest(void)
{
//	extern void ErrList_Dump(void);
	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);
	
	Printf("\r\nsID: %X%X%X\r\n",sn2,sn1,sn0);
//	Printf("\tBleMac:%s; FwVer:[%s]\n"	, g_pBle->macValid ? g_Settings.sID : "fail", g_pBle->m_FwVer);
//	Printf("\tFlash:%s\n"		, g_Settings.isFlashOk ? "pass" : "fail");
//	Printf("\tGyroscope:%s\n"	, g_Settings.isGyroOk  ? "pass" : "fail");
//	Printf("\tSY6990:%s\n"	, g_Settings.is6990Ok  ? "pass" : "fail");
//	if(!g_pGps->sateInview)
//		Printf("\tGps:searching...\n");
//	else 
//		Printf("\tGps:pass; sateInview=%d\n", Gps_GetSateInview());
//	Printf("\tSimHw:%s\n"		, g_pSimCard->isCommOk ? "pass" : "fail");
//	Printf("\tSimCar:%s\n"		, g_pGprs->isReg ? "pass" : "fail");

//	Printf("\tGprsCSQ:[%d]\n", g_pSimCard->csq);
//	Printf("\tPmsComm:[%d]\n", g_pPms->m_isCommOk);
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

///*!
// * \brief 复位
// *		  
// * \param  NONE 
// *
// * \return NONE 
// */
//static void Reset(void)
//{
//    Boot(False);
//}
//MSH_CMD_EXPORT(Reset, Reboot System);


////#endif
