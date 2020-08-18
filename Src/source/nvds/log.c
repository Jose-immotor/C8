/*
 * File      : log.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * SPI FLASH
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-14     lane      first implementation
 */
#include "common.h"
#include "Log.h"
#include "nvds.h"
#include "record.h"

static LogItem g_LogItem;
Record g_LogRecord;

const static LogEventMap g_pStrLogEvent[] = 
{
	 {MD_SYS, ET_SYS_RESET			, "SYS_RESET"}
	,{MD_SYS, ET_SYS_SLEEP			, "SYS_SLEEP"}
	,{MD_SYS, ET_SYS_WAKEUP			, "SYS_WAKEUP"}
	,{MD_SYS, ET_SYS_ACTIVE			, "SYS_ACTIVE"}
	,{MD_SYS, ET_SYS_INACTIVE		, "SYS_INACTIVE"}
	,{MD_SYS, ET_SYS_DISCHARGE_ON	, "SYS_DISCHARGE_ON"}
	,{MD_SYS, ET_SYS_DISCHARGE_OFF	, "SYS_DISCHARGE_OFF"}
	,{MD_SYS, ET_SYS_ALARM_MODE_ON	, "SYS_ALARM_MODE_ON"}
	,{MD_SYS, ET_SYS_ALARM_MODE_OFF	, "SYS_ALARM_MODE_OFF"}
//	,{MD_SYS, ET_SYS_SIGN_OK		, "SYS_SIGN_OK"}
	,{MD_SYS, ET_SYS_SIGN_FAILED	, "SYS_SIGN_FAILED"}
	,{MD_SYS, ET_TAKE_APART	, "SYS_TAKE_APART"}
	
	,{MD_SIM, ET_SIM_PWR_ON		 , "SIM_PWR_ON"}
	,{MD_SIM, ET_SIM_SLEEP		 , "SIM_SLEEP"}
	,{MD_SIM, ET_SIM_WAKEUP		 , "SIM_WAKEUP"}
	
	,{MD_SIM, ET_GPRS_CNT		 , "GPRS_CNT"}
	,{MD_SIM, ET_GPRS_DIS_CNT	 , "GPRS_DIS_CNT"}
	,{MD_SIM, ET_GPRS_SEND_FAILED, "GPRS_SEND_FAILED"}
	,{MD_SIM, ET_GPRS_HEARBEAT	 , "GPRS_HEARBEAT"}
	,{MD_SIM, ET_GPRS_SMS		 , "GPRS_SMS"}
	,{MD_SIM, ET_GPRS_UPG_START	 , "GPRS_UPG_START"}
	,{MD_SIM, ET_GPRS_UPG_PROC	 , "GPRS_UPG_PROC"}
	
	,{MD_SIM, ET_GPS_PWR_ON 	, "GPS_PWR_ON"}
	,{MD_SIM, ET_GPS_PWR_OFF	, "GPS_PWR_OFF"}
	,{MD_SIM, ET_GPS_LOC_OK		, "GPS_LOC_OK"}
	,{MD_SIM, ET_GPS_LOC_FAILED	, "GPS_LOC_FAILED"}
	
	,{MD_BLE, ET_BLE_CNT		, "BLE_CNT"}
	,{MD_BLE, ET_BLE_DIS_CNT	, "BLE_DIS_CNT"}
	
	,{MD_GRYO, ET_PMS_ACC_ON		, "ACC_ON"}
	,{MD_GRYO, ET_PMS_ACC_OFF		, "ACC_OFF"}
	,{MD_GRYO, ET_PMS_BAT_PLUG_IN	, "BAT_PLUG_IN"}
	,{MD_GRYO, ET_PMS_BAT_PLUG_OUT	, "BAT_PLUG_OUT"}
	,{MD_GRYO, ET_PMS_BAT_VERIFY	, "BAT_VERIFY"}
	,{MD_GRYO, ET_PACK_STATE_CHANGED, "PMS_PACK_STATE_CHANGED"}
	,{MD_GRYO, ET_PMS_COMM_EVENT	, "PMS_COMM_EVENT"}
	,{MD_GRYO, ET_PMS_PWR_EVENT		, "PMS_PWR_EVENT"}
	,{MD_GRYO, ET_PMS_SET_DISCHARGE	, "PMS_SET_DISCHARGE"}
	,{MD_GRYO, ET_PMS_LOG			, "PMS_LOG"}
	,{MD_GRYO, ET_BMS_FAULT			, "BMS_FAULT"}
	
	,{MD_SYS, ET_LOCK				, "LOCK"}
	,{MD_SYS, ET_CABIN_LOCK			, "CABIN_LOCK"}
	
	,{MD_UPG, ET_UPGRADE_SMART_START, "UPGRADE_SMART_START"}
	,{MD_UPG, ET_UPGRADE_SMART_DONE	, "UPGRADE_SMART_DONE"}
	,{MD_UPG, ET_UPGRADE_PMS_START	, "UPGRADE_PMS_START"}
	,{MD_UPG, ET_UPGRADE_PMS_DONE	, "UPGRADE_PMS_DONE"}
};

//int Log_Find(int startInd, RecordVerifyItemFn verify, uint32 param);

const char* Log_GetEventStr(uint8 logEvent)
{
	int i = 0;

	for(i = 0; i < GET_ELEMENT_COUNT(g_pStrLogEvent); i++)
	{
		if(logEvent == g_pStrLogEvent[i].event) return g_pStrLogEvent[i].pStr;
	}
	return "No match event";
}

void Log_Dump(LogItem* item, const char* head, const char* tail)
{
	if(head == Null) head = "";
	if(tail == Null) tail = "";
	Printf("%s%s %s, 0x%02X(%02d), 0x%02X(%02d)%s", head
		, DateTime_ToStr(item->dateTime)
		, Log_GetEventStr(item->eventId)
		, item->param1
		, item->param1
		, item->param2
		, item->param2
		, tail
		);
}

/*!
 * \brief 复位
 *		  
 * \param  argc:参数数量;argv:参数指针的指针
 *
 * \return NONE 
 */
static void Log_DumpByCount(int argc, char**argv)
//void Log_DumpByCount(int count)
{
	int count;
	
	sscanf(&(*argv[1]), "%d", &count);
	int ind = g_LogRecord.total - count;
	if(ind < 0) ind = 0;

	Log_DumpByInd(ind, count);
}
MSH_CMD_EXPORT(Log_DumpByCount, output log);

/*!
 * \brief 复位
 *		  
 * \param  argc:参数数量;argv:参数指针的指针
 *
 * \return NONE 
 */
typedef struct _LogInfoPkt
{
	uint32 records;
	uint32 dataTime;
}LogInfoPkt;
static void Log_info(int argc, char**argv)
{
	LogItem logItem = {0};
	LogInfoPkt pRst;
	
	pRst.records = g_LogRecord.total;
	if(Log_Read(&logItem, 1, 0))
	{
		//修正时间
		pRst.dataTime = logItem.dateTime;// - SECOND_OFFSET;
	}
	
	Printf("LogInfo: [%s total=%d]\n", DateTime_ToStr(pRst.dataTime), pRst.records);
}
MSH_CMD_EXPORT(Log_info, printf log info);

void Log_DumpByInd(int ind, int count)
{
	LogItem item = {0};
	
	Record_Seek(&g_LogRecord, ind);
	
	while(Record_isValid(&g_LogRecord) && count--)
	{
		if(Record_Read(&g_LogRecord, &item, sizeof(LogItem)))
		{
			if(item.eventId == ET_SYS_RESET) Printf("\n");
			Printf("%03d:[", g_LogRecord.readPointer-1);
			Log_Dump(&item, Null, "]\n");
		}
	}
}
//MSH_CMD_EXPORT(Log_DumpByInd,printf Log Dump By Ind);
////2008/04/03 11:12:00
//void Log_DumpByTimeStr(const char* pDataTime, int count)
//{
//	int year, month, day, hour, minute, sec;
//	if(6 == sscanf(pDataTime, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &minute, &sec))
//	{
//		uint32 seconds = DateTime_GetSeconds(DateTime_MakeRtc(year, month, day, hour, minute, sec));
//		Log_DumpByTime(seconds, count);
//	}
//	else
//	{
//		Printf("Date time format error\n");
//	}
//}

//void Log_DumpByTime(uint32 seconds, int count)
//{	
//	int ind = Log_Find(0, (RecordVerifyItemFn)Log_FilterTime, seconds);

//	Log_DumpByInd(ind, count);
//}

static Bool g_LogAllowAdd = True;
void Log_Add(LogEvent logEvent, LogType logType, uint8 param1, uint8 param2)
{
	if(!g_LogAllowAdd || g_LogRecord.sectorCount == 0/*还没有被初始化*/) return;
	
	LogItem* item = &g_LogItem;
	g_LogItem.dateTime = DateTime_GetSeconds(Null);
	g_LogItem.version = LOG_DATA_VER;
	g_LogItem.eventId = logEvent;
	g_LogItem.logType = logType;
	g_LogItem.param1 = param1;
	g_LogItem.param2 = param2;

	if(g_dwDebugLevel & DL_LOG)
	{
		Printf("Log[%d][%s %s, 0x%02X(%02d), 0x%02X(%02d)]\n", g_LogRecord.total
			, DateTime_ToStr(item->dateTime)
			, Log_GetEventStr(item->eventId)
			, item->param1
			, item->param1
			, item->param2
			, item->param2
			);
	}
	
	Record_Write(&g_LogRecord, &g_LogItem);
}

Bool Log_VerifyItem(LogItem* pItem, uint32 param)
{
	return (LOG_DATA_VER == pItem->version);	//校验版本号
}

//int Log_Find(int startInd, RecordVerifyItemFn verify, uint32 param)
//{
//	int i = 0;
//	LogItem item = {0};

//	Record_Seek(&g_LogRecord, startInd);
//	
//	for(i = startInd; i < g_LogRecord.total; i++)
//	{
//		if(Record_Read(&g_LogRecord, &item, sizeof(LogItem)))
//		{
//			if(verify(&item, param))
//			{
//				return i;
//			}
//		}
//		else
//		{
//			break;
//		}
//	}

//	return LOG_INVALID_IND;
//}

//Bool Log_FilterTime(LogItem* pItem, uint32 param)
//{
//	return pItem->dateTime >= param;
//}

//Bool Log_FilterEvent(LogItem* pItem, uint8 events[], int count)
//{
//	for(int i = 0; i < count; i++)
//	{
//		if(pItem->eventId == events[i])
//		{
//			return True;
//		}
//	}
//	
//	return False;
//}

//int Log_ReadEx(LogItem* pLogItem, int count, uint8* readSectorOffset, int* ind)
//{
//	//当前所有扇区都有内容 
//	int logInd = * ind;
//	int i = 0;
//	
//	Record_ConvertSector(&g_LogRecord, readSectorOffset, ind);
//	logInd = *ind;
//	
//	Record_Seek(&g_LogRecord, logInd);
//	while(Record_isValid(&g_LogRecord) && i < count)
//	{
//		Record_Read(&g_LogRecord, &pLogItem[i++], sizeof(LogItem));
//	}
//	
//	*ind = g_LogRecord.readPointer;
//	
//	return i * sizeof(LogItem);
//}

int Log_Read(LogItem* pLogItem, int count, int ind)
{
	int i = 0;
	
	Record_Seek(&g_LogRecord, ind);
	while(Record_isValid(&g_LogRecord) && i < count)
	{
		Record_Read(&g_LogRecord, &pLogItem[i++], sizeof(LogItem));
	}

	return i * sizeof(LogItem);
}

////检索minute分钟之前发生的记录
//int Log_FindByTime(uint32 seconds)
//{
//	return Log_Find(0, (RecordVerifyItemFn)Log_FilterTime, seconds);
//}

//int Log_ReadByEvents(uint8* pBuf, int len, int ind, uint8 events[], int count)
//{
//	int copyLen = 0;
//	LogItem logItem = {0};
//	
//	Record_Seek(&g_LogRecord, ind);
//	while(Record_isValid(&g_LogRecord) && (len-copyLen) >= sizeof(LogItem))
//	{
//		if(Record_Read(&g_LogRecord, &logItem, sizeof(LogItem)) && Log_FilterEvent(&logItem, events, count))
//		{
//			memcpy(&pBuf[copyLen], &logItem, sizeof(logItem));
//			copyLen += sizeof(LogItem);
//		}
//	}

//	return copyLen;
//}

//int Log_GetRemain(uint8 readSec, int pos)
//{
//	Record_ConvertSector(&g_LogRecord, &readSec, &pos);
//	return (g_LogRecord.total - pos);
//}

//int Log_GetLogPkt(GprsLogPkt* pGprsLogPkt, int count, uint8* readSec, int* pos)
//{
//	int len = 0;
//	LogItem* logItem = (LogItem*)&g_CommonBuf[256];
//	
//	Record_ConvertSector(&g_LogRecord, readSec, pos);
//	if(g_LogRecord.total - *pos < count) return 0;
//	
//	Log_ReadEx(logItem, count, readSec, pos);
//	
//	for(int i = 0; i < count; i++)
//	{
//		if(!Log_VerifyItem(&logItem[i], 0))
//		{
//			return 0;	//测试是否存在非法数据记录
//		}
//		
//		memcpy(&pGprsLogPkt[i], &logItem[i].dateTime, sizeof(GprsLogPkt));
//		pGprsLogPkt[i].dateTime -= SECOND_OFFSET;	//修正时间，比服务器的时间快了8小时
//		
//		len += sizeof(GprsLogPkt);
//	}
//		
//	//Printf("%s:(%d - %d)\n", DateTime_ToStr(seconds), logInd, len);
//	return len;
//}

void Log_Init()
{
	g_LogItem.version = LOG_DATA_VER;
	Record_Init(&g_LogRecord,&g_LogItem,(void*)g_pNvdsMap->logBuf, 
		sizeof(g_LogItem),MAX_LOG_SECTOR,(RecordVerifyItemFn)Log_VerifyItem);
}

//void Log_SetPostInd(int ind, int count)
//{
//	extern  Bool g_isPostEnable;
//	g_SysCfg.readLogStartSector = g_LogRecord.readStartSectorInd;
//	Record_ConvertSector(&g_LogRecord, &g_SysCfg.readLogStartSector, &ind);
//	g_SysCfg.postLogInd = ind;
//	g_SysCfg.readLogCount = count;
//	g_isPostEnable = True;
//	
//	Nvds_Write_SysCfg();
//	Printf("Set sector=%d, ind=%d, count=%d\n", g_SysCfg.readLogStartSector, g_SysCfg.postLogInd, count);
//}

//void Log_AddTest(int count, Bool allowAdd)
//{
//	static int seconds = 1527509100;
//	LogItem logItem = {0};
//	
//	Printf("Log_AddTest, count=%d, allowAdd=%d\n", count, allowAdd);
//	g_LogAllowAdd = allowAdd;

//	if(count)
//	{
//		for(int i = 0; i < count; i++)
//		{
//			logItem.dateTime = seconds++;
//			logItem.eventId = ET_SYS_WAKEUP;
//			logItem.version = LOG_DATA_VER;

//			Record_Write(&g_LogRecord, &logItem);
//		}
//	}
//	
//	Printf("Log ReadStartSectorInd=%d, total=%d\n", g_LogRecord.readStartSectorInd, g_LogRecord.total);
//}

void Log_RemoveAll()
{
	Record_RemoveAll(&g_LogRecord);
	g_SysCfg.postLogInd = 0;
	g_SysCfg.readLogStartSector = 0;
	Nvds_Write_SysCfg();
}
////#ifdef LOG_TESTER
////#include "LogTester.c"
////#endif
//#endif

