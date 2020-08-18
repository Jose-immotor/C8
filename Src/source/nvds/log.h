#ifndef __LOG_H_
#define __LOG_H_

#ifdef __cplusplus
extern "C"{
#endif



#include "Record.h"
#include "time.h"

#define LOG_INVALID_IND 0xFFFFFFFF

//typedef Bool (*LogItemFilterFn)(LogItem)
#define SECOND_OFFSET 28800
typedef enum _ModType
{
 	 MD_SYS
	,MD_SIM
	,MD_BLE
	,MD_GRYO
	,MD_FLASH
	,MD_UPG
}ModType;

typedef enum _LogType
{
	 LT_INFO
	,LT_ALARM
}LogType;

typedef enum _LogEvent
{
	//system event
	 ET_SYS_RESET = 1
	,ET_SYS_SLEEP 			//(RemainMin, Round5V())
	,ET_SYS_WAKEUP			//(devcfg, WakeupType)
	,ET_SYS_ACTIVE			//(devcfg, 0)
	,ET_SYS_INACTIVE		//(devcfg, 0)
	,ET_SYS_DISCHARGE_ON	//(devcfg, 0)
	,ET_SYS_DISCHARGE_OFF	//(devcfg, 0)
	,ET_SYS_ALARM_MODE_ON	//(devcfg, 0)
	,ET_SYS_ALARM_MODE_OFF	//(devcfg, 0)
//	,ET_SYS_SIGN_OK
	,ET_SYS_SIGN_FAILED		//(devcfg, 0)
	,ET_TAKE_APART			//拆开检测
	
	//SIM event
	,ET_SIM_PWR_ON = 20		//(reason, CSQ)
	,ET_SIM_SLEEP			//(HbCount, GpsCount)
	,ET_SIM_WAKEUP			//(devcfg, Round5V())
	
	//GPRS event
	,ET_GPRS_CNT = 30		//(RemainMin, CSQ)
	,ET_GPRS_DIS_CNT		//(reason, CSQ)
	,ET_GPRS_SEND_FAILED	//(Round5V(), CSQ)
	,ET_GPRS_HEARBEAT		//(HbCount, GpsCount)
	,ET_GPRS_SMS			//(ind, 0)
	,ET_GPRS_UPG_START		//(Pms_IsAccOn, (g_SmartUpgFlag<<1) | g_PmsUpgFlag)
	,ET_GPRS_UPG_PROC		//(step, var)
	
	//GPS event
	,ET_GPS_PWR_ON = 40		//(Round5V(), CSQ)
	,ET_GPS_PWR_OFF			//(Round5V(), CSQ)
	,ET_GPS_LOC_OK			//(satelies, snr)
	,ET_GPS_LOC_FAILED		//(satelies, snr)
	
	//BLE event
	,ET_BLE_CNT = 50
	,ET_BLE_DIS_CNT
	
	//GYRO event
	,ET_PMS_ACC_ON = 60		//(RemainMin, SOC)
	,ET_PMS_ACC_OFF			//(RemainMin, SOC)
	,ET_PMS_BAT_PLUG_IN		//(battry count, SOC)
	,ET_PMS_BAT_PLUG_OUT	//(battry count, SOC)
	,ET_PMS_BAT_VERIFY		//(isOk, SOC)
	,ET_PACK_STATE_CHANGED	//(packState, SOC)
	,ET_PMS_COMM_EVENT		//(CommState, SOC),通信中断
	,ET_PMS_PWR_EVENT		//(PWR, SOC),PMS上电/关电
	,ET_PMS_SET_DISCHARGE	//(isDiacharge, (Ctrl<<2 | Port0)<<2) | Port1),设置PMS放电模式
	,ET_PMS_LOG				//(Pms Parma 1, Pms Parma 2)
	,ET_BMS_FAULT			//(isFault, batPort Bit)
	
	,ET_LOCK				//(0, isLock)
	,ET_CABIN_LOCK			//(0, isLock)
			
	//UPGRADE event
	,ET_UPGRADE_SMART_START = 80	//(Round5V())
	,ET_UPGRADE_SMART_DONE			//(Round5V(), SOC)
	,ET_UPGRADE_PMS_START			//(Round5V())
	,ET_UPGRADE_PMS_DONE			//(Round5V(), SOC)
	
}LogEvent;

typedef struct _LogEventMap
{
	ModType  modType;
	LogEvent event;
	const char* pStr;
}LogEventMap;

#define LOG_DATA_VER 1	//注意: 修改该版本号，会导致删除以前保存在Flash的所有数据。
#pragma pack(1) 
typedef struct _LogItem
{	
	uint8 version:3;	//第一个Field必须是版本号
	uint8 reserved:3;
	uint8 logType:2;
	time_t dateTime;
	uint8 eventId;
	uint8 param1;
	uint8 param2;
}LogItem;

typedef struct _GprsLogPkt
{
	time_t dateTime;
	uint8 eventId;
	uint8 param1;
	uint8 param2;
}GprsLogPkt;

#pragma pack() 

extern Record g_LogRecord;

Bool Log_FilterTime(LogItem* pItem, uint32 param);
Bool Log_FilterEvent(LogItem* pItem, uint8 events[], int count);

void Log_Init(void);
void Log_Dump(LogItem* item, const char* head, const char* tail);
void Log_Add(LogEvent logEvent, LogType logType, uint8 param1, uint8 param2);
int Log_Read(LogItem* pLogItem, int count, int ind);
//void Log_DumpByCount(int count);
void Log_DumpByInd(int ind, int count);
void Log_DumpByTime(uint32 seconds, int count);
void Log_Tester1(void);
void Log_DumpByTimeStr(const char* pDataTime, int count);
int Log_FindByTime(uint32 seconds);
void Log_RemoveAll(void);
int Log_GetLogPkt(GprsLogPkt* pGprsLogPkt, int count, uint8* readSec, int* pos);
int Log_ReadEx(LogItem* pLogItem, int count, uint8* readSectorOffset, int* ind);
void Log_AddTest(int count, Bool allowAdd);
void Log_SetPostInd(int ind, int count);
void Log_Tester(void);
int Log_GetRemain(uint8 readSec, int pos);

#define LOG1(x) 			Log_Add(x, LT_INFO, 0, 0);
#define LOG2(x,x1,x2) 		Log_Add(x, LT_INFO, (x1), (x2));


void Log_Tester(void);

#ifdef __cplusplus
}
#endif

#endif

