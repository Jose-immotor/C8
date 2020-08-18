/****************************************************************************//**
 * @file    ble.c
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 2016-5-31 $
 * @brief   BLE operation API implement file .c
 *
 * @note
 * Copyright (C) 2016 immotor Technology Corp. All rights reserved.
*****************************************************************************/

#include "common.h"
#include "nvds.h"
#include "smart_system.h"
#include "errordef.h"
//#include "datarom.h"

////#include "Nvds.h"
//#define  USER_NUM     5

//#define PI                      3.1415926
//#define EARTH_RADIUS            6378.137        //地球近似半径


///*---------------------------------------------------------------------------------------------------------*/
///* Global variables                                                                                        */
///*---------------------------------------------------------------------------------------------------------*/


////UserInfo  usersList[USER_NUM]; 
//uint8_t  currentUserIndex;

ScooterSettings  g_Settings;
ErrorCode g_errorInfo;

static uint8  g_errorList[ERR_TYPE_MAX];


//void SetSignEn(Bool isEnable)
//{
//	g_Settings.IsSignEnable = isEnable; 
//	Beep_Mode(BEEP_DEV_SIGN_EN);
//	LOG2(isEnable ? ET_SYS_DISCHARGE_ON : ET_SYS_DISCHARGE_OFF, g_Settings.devcfg, 0);
//}

//void SetForbidDischarge(Bool isEnable)
//{
//	g_Settings.IsForbidDischarge = isEnable; 
//	Beep_Mode(BEEP_DEV_DISCHARGE_EN);
//	LOG2(isEnable ? ET_SYS_DISCHARGE_OFF : ET_SYS_DISCHARGE_ON, g_Settings.devcfg, 0);
//}

Bool IsActive(){return !g_Settings.IsDeactive;}
//Bool IsSignEn(){return g_Settings.IsSignEnable;}
//Bool IsForbidDischarge(){return g_Settings.IsForbidDischarge;}
//void SetActive(Bool isActive)
//{
//	g_Settings.IsDeactive = !isActive; 
//	Beep_Mode(BEEP_DEV_ACTIVE);
//	LOG2(isActive ? ET_SYS_ACTIVE : ET_SYS_INACTIVE, g_Settings.devcfg, 0);
//}

//void ErrList_Dump()
//{
//	 const static char* pErrName[] = 
//	 {
//	 	 "SYSTEM"
//		,"MOTOR1"
//		,"MOTOR2"
//		,"BATTERY1"
//		,"BATTERY2"
//		,"GPRS"
//		,"GPS"
//		,"BT"
//		,"GROY"
//		,"MAX"
//	 };
//	int i = 0;
//	Bool isPrintf = False;
//	
//	rt_kprintf("Dump error list:\n", i, g_errorList[i]);
//	for(i = 0; i < ERR_TYPE_MAX; i++)
//	{
//		if(g_errorList[i])
//		{
//			isPrintf = True;
//			rt_kprintf("\t Err[%s]=0x%X(%d)\n", pErrName[i], g_errorList[i], g_errorList[i]);
//		}
//	}
//	if(!isPrintf)
//		rt_kprintf("\t No error\n");
//}

// /**/
// void SetErrorCode(uint8 err_type, uint32 errcode, ErrType errType)
// {
// 	if(err_type >= ERR_TYPE_MAX)
//		return;
//	
////	if(err_type == ERR_TYPE_GPRS) rt_kprintf("Err[%d][0x%x].\n", err_type, errcode);
// 	g_errorList[err_type] = (ET_ALARM == errType || errcode==0) ? errcode : (errcode | 0x80);
// }

// Bool ErrorCode_IsAlarm(uint8 errcode)
// {
// 	return (errcode & 0x80) == 0;
// }

// uint8 GetErrorCode(uint8 err_type)
// {
// 	if(err_type >= ERR_TYPE_MAX)
//		return 0;
//	
// 	return g_errorList[err_type];
// }

///*显示在LCD上的故障码，10进制*/
// uint8 GetErrorCodeDec(void)
// {
//	return 0;
// }

// void setAlarmCode(uint8 alarm)
// {
// }


//// 求弧度
//double radian(double d)
//{
//    return d * PI / 180.0;   //角度1? = π / 180
//}

////计算距离单位KM
//double get_distance(double lat1, double lng1, double lat2, double lng2)
//{
//    double radLat1 = radian(lat1);
//    double radLat2 = radian(lat2);
//    double a = radLat1 - radLat2;
//    double b = radian(lng1) - radian(lng2);

//    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));

//    dst = dst * EARTH_RADIUS;
//    dst= round(dst * 10000) / 10000.0;
//	
//    return dst;
//}


////本地MD5密钥是否匹配
//uint8_t IsUserValid(char*userID)
//{
//	static uint8 count = 0;
//	uint8 valid=FALSE;

//	if(memcmp(userID, g_Settings.localKey.admin, MD5_KEY_LEN) == 0)
//	{
//		currentUserIndex= 0;
//		valid = TRUE;
//	}
//	else if(memcmp(userID,g_Settings.localKey.guest, MD5_KEY_LEN) == 0)
//	{
//		currentUserIndex= 1;
//		valid = TRUE;	
//	}
//	else if(count == 0)	//重新计算MD5并且重新校验,主要用于BLE模组被更换过的情况
//	{
//		Ble_GetMd5(g_Settings.sID, MD5_ADMIN, g_Settings.localKey.admin);
//		Ble_GetMd5(g_Settings.sID, MD5_GUEST, g_Settings.localKey.guest);
//		count = 1;
//		valid = IsUserValid(userID);
//		if(valid)
//		{
//			Nvds_Write_Md5Key();
//		}
//	}

//	//PFL(DL_MAIN, "AUTH[%d]: %s admin:%s\n", count, userID, g_Settings.localKey.admin);

//	count = 0;
//	return valid;
//}

////VerDesc* GetPmsVerDesc(void)
////{
////	//g_Settings.pmsVer.m_AppBuildeNum = 1;
//////	return (VerDesc*)&g_Settings.pmsVer;
////}

//void WritePmsFileLen(uint32 fielLen)
//{
//	g_Settings.pmsFwLen = fielLen;
//	Nvds_Write_Misc();
//}
//uint32 GetPmsFileLen(void)
//{
//	return g_Settings.pmsFwLen;
//}

//uint8_t getCurrentUserIndex(void)
//{
//	return currentUserIndex;
//}

//char* Sys_GetSID(void)
//{
//	return g_Settings.sID;
//}


///*锁车管理*/

//void UpdateLock(uint8_t isLock, Bool isSendMsg)
//{
//}


void Smart_SettingsInit(void)
{
	memset(&g_Settings, 0, sizeof(g_Settings));
	memset(&g_errorList, 0, sizeof(g_errorList));

	g_Settings.settingVer = SETTING_VER;
	g_Settings.miscVer 	  = MISC_VER;

	Nvds_SettingDefault();
}

Bool IsAlarmMode()
{
	//return (!g_Settings.IsDeactive) && (g_Settings.IsForbidDischarge || g_Settings.IsAlarmMode);
	return g_Settings.IsAlarmMode;
}
