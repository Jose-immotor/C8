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

//�ı�洢�ṹ����Ҫ������ǰ���������ʱ����Ҫ�ı�汾��
#define MISC_VER 		3
#define SETTING_VER 	4
#define MOVETRACK_VER 	2
#define LOCK_TEST		0x5A

#define CUTOFF_STOP 	0x00
#define CUTOFF_ATONCE	0x01

#define STOP_TIME_DEFAULT   300
#define STOP_CURRENT		2500

//�ϵ�ģʽ
#define POWER_CUTOFF_STOP   0x01   //ͣ���ϵ�
#define POWER_CUTOFF_NOW    0x02   //���̶ϵ�

#pragma anon_unions
#pragma pack(1) 
typedef struct 
{	
	uint8 mac[6];
	Bool  isFlashOk; //Flash is OK;
	Bool  isGyroOk;  //������ is OK;
	Bool  is6990Ok;  //SY6990 is OK;

	/*Settings*/
	uint8 settingVer;	//settin�汾�ţ�������ڵ�һ���ֽ�,����뱣���settinVer�汾�Ų�һ�£��ᵼ��ɾ����������ݶ�ʹ��Ĭ��ֵ
	
	uint8 isLockTest:1;	//������
	uint8 isRemoteAccOn:1;	//Զ�̿���ACC
	uint8 isTakeApart:1;	//�𿪼��
	uint8 reserved10:5;
	
	uint8 isLock:1;		//�Ƿ�����
	uint8 isGyroAssert:1;	//�������춯
	uint8 isMoving:1;		//�Ƿ����ƶ�
	uint8 isVolValid:1;
	uint8 canNotUsed:1;		//��λ���ܱ�ʹ�ã�Ĭ��ֵΪ1
	uint8 reserved11:3;
#ifdef CFG_NVC	
	uint8 LoSocPlayFlag[3];	//�͵���������־
	uint16 SocPlayFlag;		//����������־
#endif
	union 
	{
		uint8 devcfg;
		struct
		{
		uint8 IsDeactive:1;			//�Ƿ�ȥ����,������ǰ�汾
		uint8 IsSignEnable:1;	 	//�ǼǴ򿨹���ʹ��
		uint8 IsForbidDischarge:1;	//�Ƿ��ֹ�ŵ�
		uint8 IsAlarmMode:1;		//�Ƿ񾳽�ģʽ
		uint8 IsBatVerifyEn:1;		//�Ƿ�ʹ�ܵ�������֤
		uint8 reserved01:3;
		};
	};
	uint8 vol;					//����
	
#ifdef CFG_LOGIN_TIME	


	uint32 loginMaxMinute;		//���ʱ�䣬����Ϊ��λ,
	uint32 remainSeconds;		//��ʣ��ʱ�䣬����Ϊ��λ,
	uint8 vol;					//����
	//uint16 StopTime;			//ͣ���ж�����-С��������ʱ��
	//uint16 StopCurrent;			//ͣ���ж�����-С������ֵ
	uint8 reserved1[6];//����
#else
	uint8 reserved1[16];//����
#endif
	uint8 settingEnd;	//setting�洢����λ��,��ֵû�����壬������λ����Ϣ�����㱣������������С,������Flash��
	
	//misc sector
	uint8 miscVer;		//misc�汾�ţ�������ڵ�һ���ֽ�,����뱣���miscVer�汾�Ų�һ�£��ᵼ��ɾ����������ݶ�ʹ��Ĭ��ֵ��
	char sID[SID_LEN+1];
	MD5_Key localKey;	//������Flash������һ������
	
	uint8 isModifiedMfInfo:1;
	uint8 isGetBtMac:1;
	uint8 reserved2:6;
	
	VerDesc pmsVer;		//����ֱ������Ϊ�ṹ���ͣ����������ֽڶ�������
	uint32  pmsFwLen;	//pms�̼�����,0��ʾû�д����µĹ̼�
	uint8 	isBleUpg;	//�Ƿ����������̼�
	uint32 	bmsFwLen;	//bms�̼�����
	uint8 reserved3[7];//����
	uint8 miscEnd;		//misc�洢����λ��,��ֵû�����壬������λ����Ϣ�����㱣������������С,������Flash��
	
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
	,ET_FAULT	//���ϣ�LCD����ʾ"!"
	,ET_ALARM	//�澯
}ErrType;
void SetErrorCode(uint8 err_type, uint32 errcode, ErrType errType);

 uint8 GetErrorCode(uint8 err_type);

 //��ȡ������ȼ�������ʾ
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



