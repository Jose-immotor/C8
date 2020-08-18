/****************************************************************************//**
 * @file    ble.h
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 2016-5-31 $
 * @brief   BLE operation header file
 *
 * @note
 * Copyright (C) 2016 immotor Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __SMART_SYSTEM_H__
#define __SMART_SYSTEM_H__

//#include "commondef.h"
#include "datarom.h"
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define USERID_LEN  16
#define SID_LEN		12
#define CCID_LEN  	20
#define MD5_KEY_LEN 16

//typedef struct 
//{
//	uint8 type; //0x00: invalid, 0x01: admin, 0x02:guest, 
//	char userID[USERID_LEN+1];	
//}UserInfo;

typedef struct
{
	char admin[MD5_KEY_LEN+1];
	char guest[MD5_KEY_LEN+1];
}MD5_Key;

typedef struct
{
	char iccid[CCID_LEN+1];
	uint16 CC;  //Country Code
	uint8  csq;
}SimCard_Info;

typedef struct
{
	uint8 maxSpeed;
}ModeSetting;


typedef struct
{
	uint8 systemError;
	uint8 gpsError;	
	uint16 motorError;
	uint32 batteryError;
	uint8 gprsError;
	uint8 peripheralError;
	uint8 reserve;
	uint8 lcdErrorcode;
}ErrorCode;

#define MAX_APN_SIZE   20


#define MODE_BOOST		0
#define MODE_NORMAL1	1
#define MODE_NORMAL2	2
#define MODE_POWER_ASSISTED 	3

#define MODE1_MAX_SPEED_USA  6
#define MODE1_MAX_SPEED_CHN  10

//改变存储结构，需要舍弃以前保存的数据时则需要改变版本号
#define MISC_VER 		3
#define SETTING_VER 	4
#define MOVETRACK_VER 	2
#define LOCK_TEST		0x5A

#define CUTOFF_STOP 	0x00
#define CUTOFF_ATONCE	0x01

#define STOP_TIME_DEFAULT   300
#define STOP_CURRENT		2500

//断电模式
#define POWER_CUTOFF_STOP   0x01   //停车断电
#define POWER_CUTOFF_NOW    0x02   //立刻断电

#pragma anon_unions
#pragma pack(1) 
typedef struct 
{	
	uint8 mac[6];
	Bool  isFlashOk; //Flash is OK;
	Bool  isGyroOk;  //陀螺仪 is OK;
	Bool  is6990Ok;  //SY6990 is OK;

	/*Settings*/
	uint8 settingVer;	//settin版本号，必须放在第一个字节,如果与保存的settinVer版本号不一致，会导致删除保存的数据而使用默认值
	
	uint8 isLockTest:1;	//锁测试
	uint8 isRemoteAccOn:1;	//远程控制ACC
	uint8 isTakeApart:1;	//拆开检测
	uint8 reserved10:5;
	
	uint8 isLock:1;		//是否锁车
	uint8 isGyroAssert:1;	//陀螺仪异动
	uint8 isMoving:1;		//是否在移动
	uint8 isVolValid:1;
	uint8 canNotUsed:1;		//该位不能被使用，默认值为1
	uint8 reserved11:3;
#ifdef CFG_NVC	
	uint8 LoSocPlayFlag[3];	//低电量播报标志
	uint16 SocPlayFlag;		//电量播报标志
#endif
	union 
	{
		uint8 devcfg;
		struct
		{
		uint8 IsDeactive:1;			//是否去激活,兼容以前版本
		uint8 IsSignEnable:1;	 	//登记打卡功能使能
		uint8 IsForbidDischarge:1;	//是否禁止放电
		uint8 IsAlarmMode:1;		//是否境界模式
		uint8 IsBatVerifyEn:1;		//是否使能电池身份验证
		uint8 reserved01:3;
		};
	};
	uint8 vol;					//音量
	
#ifdef CFG_LOGIN_TIME	


	uint32 loginMaxMinute;		//打卡最长时间，分钟为单位,
	uint32 remainSeconds;		//打卡剩余时间，分钟为单位,
	uint8 vol;					//音量
	//uint16 StopTime;			//停车判定条件-小电流持续时间
	//uint16 StopCurrent;			//停车判定条件-小电流阈值
	uint8 reserved1[6];//保留
#else
	uint8 reserved1[16];//保留
#endif
	uint8 settingEnd;	//setting存储结束位置,该值没有意义，仅用其位置信息，计算保存的数据区块大小,不会存进Flash中
	
	//misc sector
	uint8 miscVer;		//misc版本号，必须放在第一个字节,如果与保存的miscVer版本号不一致，会导致删除保存的数据而使用默认值。
	char sID[SID_LEN+1];
	MD5_Key localKey;	//保存在Flash的另外一个扇区
	
	uint8 isModifiedMfInfo:1;
	uint8 isGetBtMac:1;
	uint8 reserved2:6;
	
	VerDesc pmsVer;		//不能直接声明为结构类型，否则会存在字节对齐问题
	uint32  pmsFwLen;	//pms固件长度,0表示没有待更新的固件
	uint8 	isBleUpg;	//是否蓝牙升级固件
	uint32 	bmsFwLen;	//bms固件长度
	uint8 reserved3[7];//保留
	uint8 miscEnd;		//misc存储结束位置,该值没有意义，仅用其位置信息，计算保存的数据区块大小,不会存进Flash中
	
}ScooterSettings;
#pragma pack() 
extern  ScooterSettings  g_Settings;

/*
Get Scooter ID, ==BD_ADDR now
*/
char* Sys_GetSID(void);

uint8 getCurrentUserIndex(void);


uint8 IsUserValid(char*userID);

void UpdateLock(uint8 isLock, Bool isSendMsg);

uint8 getLockState(void);

uint8 IsAdmin(char*userID);

double radian(double d);
double get_distance(double lat1, double lng1, double lat2, double lng2);

typedef enum _ErrType
{
	 ET_NONE = 0
	,ET_FAULT	//故障，LCD上显示"!"
	,ET_ALARM	//告警
}ErrType;
void SetErrorCode(uint8 err_type, uint32 errcode, ErrType errType);

 uint8 GetErrorCode(uint8 err_type);

 //获取最高优先级错误显示
 uint8 GetErrorCodeDec(void);

 void setAlarmCode(uint8 alarm);


void Smart_SettingsInit(void);

void Smart_powerDown(void);
VerDesc* GetPmsVerDesc(void);
uint32 GetPmsFileLen(void);
void WritePmsFileLen(uint32 fielLen);

//void Smart_wakeUpCtr(void);
void SetActive(Bool isActive);
void SetSignEn(Bool isEnable);
void SetForbidDischarge(Bool isEnable);
Bool IsActive(void);
Bool IsSignEn(void);
Bool IsForbidDischarge(void);
Bool IsAlarmMode(void);

#define ERROR_TYPE_NUM  10

#ifdef __cplusplus
}
#endif

#endif

/*** (C) COPYRIGHT 2016 immotor Technology Corp. ***/



