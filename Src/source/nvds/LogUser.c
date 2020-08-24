
#include "LogUser.h"
#include "nvds.h"
//#include "LogModule.h"
#include "CcuDef.h"
#include "CcuLog.h"
#include "PmsLog.h"
#include "ShelfLockLog.h"
#include "MeterLog.h"
#include "log.h"

LogItem* g_pLogItem;
static Record g_LogRecord;

Bool LogUser_IsPrintf(const LogItem* p)
{
	if (g_pDbgInfo->pmsAddrForPrintf)
	{
		return (g_pDbgInfo->pmsAddrForPrintf == p->moduleId);
	}
	return True;
}

static void Record_LogUser_Dump(int argc, char**argv)
{
	Record_Dump(&g_LogRecord);
}
MSH_CMD_EXPORT(Record_LogUser_Dump, output Record);

const LogEventMap* Log_GetLogEventMap(const LogEventMap* pLogEventMap, int count, const LogItem* pItem)
{
	for (int i = 0; i < count; i++, pLogEventMap++)
	{
		if (pLogEventMap->catId == pItem->catId
			&& pLogEventMap->subId == pItem->subId 
			&& pLogEventMap->eventId == pItem->eventId)
		{
			return pLogEventMap;
		}
	}

	return Null;
}

//static LogModule g_LogModule[1];
const static LogModule g_LogModule[] =
{
	{CCU_ADDR		 , CCU_ADDR			, Ccu_EventIdToStr	    , Ccu_EventValueToStr},
//	{PMS_ADDR_0		 , PMS_ADDR_12		, Pms_EventIdToStr	    , Pms_EventValueToStr, Pms_ModuleValueToStr},
//	{METER_ADDR		 , METER_ADDR		, Meter_EventIdToStr    , Meter_EventValueToStr},
//	{SHELF_LOCK_ADDR , SHELF_LOCK_ADDR	, ShelfLock_EventIdToStr, ShelfLock_EventValueToStr},
};

const LogModule* Get_ModuleDispatch(const LogItem* pItem)
{
	const LogModule* p = g_LogModule;
	for (int i = 0; i < GET_ELEMENT_COUNT(g_LogModule); i++, p++)
	{
		if (pItem->moduleId >= p->moduleIdMin && pItem->moduleId <= p->moduleIdMax)
		{
			return p;
		}
	}
	return Null;
}

const char* __attribute__((weak)) DateTime_FmtStr(uint32 seconds)
{
	//2019-12-16 11:32:20
	static char timeStr[20];
	struct tm* pTm = localtime((time_t*)&seconds);
	strftime(timeStr, 20, "%Y-%m-%d %H:%M:%S", pTm);
	//remove year
	return &timeStr[0];
}

const char* __attribute__((weak)) LogType_ToStr(const LogItem* pItem, char* buf, int bufSize)
{
	if (pItem->logType == LT_WARNING) return "WARNING ";
	if (pItem->logType == LT_ERROR) return "ERROR ";
	return "";
}

const char* EventId_ToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogModule* p = Get_ModuleDispatch(pItem);

	if (p && p->EventIdToStr)
	{
		return p->EventIdToStr(pItem, buf, bufSize);
	}

	sprintf(buf, "%02X", pItem->eventId);
	return buf;
}

const char* ModuleId_ToStr(const LogItem* pItem, char* buf, int bufSize)
{	
	const LogModule* p = Get_ModuleDispatch(pItem);
	if(pItem->moduleId == CCU_ADDR)
	{
		strcpy(buf, "CCU");
	}
	else if (pItem->moduleId == METER_ADDR)
	{
		strcpy(buf, "METER");
	}
	else if (pItem->moduleId == SHELF_LOCK_ADDR)
	{
		strcpy(buf, "SHELF_LOCK");
	}
	else if(p && p->MoudleIdToStr)
	{
		p->MoudleIdToStr(pItem, buf, bufSize);
	}
	else
	{
		sprintf(buf, "MODULE[%d]", pItem->moduleId);
	}
	return buf;
}

const char* CatId_ToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogModule* p = Get_ModuleDispatch(pItem);

	if (p && p->CatIdToStr)
	{
		return p->CatIdToStr(pItem, buf, bufSize);
	}

	sprintf(buf, "%02X", pItem->eventId);
	return buf;
}

const char* SubId_ToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogModule* p = Get_ModuleDispatch(pItem);

	if (p && p->SubIdToStr)
	{
		return p->SubIdToStr(pItem, buf, bufSize);
	}

	sprintf(buf, "%02X", pItem->eventId);
	return buf;
}

const char* EventValue_ToStr(const LogItem* pItem, const char* fmt, char* buf, int bufSize)
{
	uint8 bites;
	char sprintfBitbuf[250]; 
	if(fmt == Null) fmt = "0x%08X (%d)";
	const uint32* pAsUint32 = (uint32*)pItem->data;

	//sample: fmt = "BOM ID[%0-7BX], FAULT[%8-15B]"
	if(SprintfBit(sprintfBitbuf, fmt, *pAsUint32, &bites))
	{
		if (bites > 24)      fmt = "0x%08X (%s)";
		else if (bites > 16) fmt = "0x%06X (%s)";
		else if (bites > 8)  fmt = "0x%04X (%s)";
		else                 fmt = "0x%02X (%s)";
		sprintf(buf, fmt, *pAsUint32, sprintfBitbuf);
	}
	else
	{
		sprintf(buf, fmt, *pAsUint32, *pAsUint32);
	}
	return buf;
}

const char* LogVal_ToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogModule* p = Get_ModuleDispatch(pItem);

	if (p && p->ValueToStr)
	{
		return p->ValueToStr(pItem, buf, bufSize);
	}
	else
	{
		return EventValue_ToStr(pItem, Null, buf, bufSize);
	}
}

//void LogUser_DumpByCount(int count, uint8 moduleId)
static void LogUser_DumpByCount(int argc, char**argv)
{
	int count;
	int moduleId;
	uint8 temp = g_pDbgInfo->pmsAddrForPrintf;

	g_pDbgInfo->pmsAddrForPrintf = moduleId;
	Record_DumpByCount(&g_LogRecord, count, moduleId);
	g_pDbgInfo->pmsAddrForPrintf = temp;
}
MSH_CMD_EXPORT(LogUser_DumpByCount, LogUser_DumpByCount);

//void LogUser_DumpByInd(int ind, int count, uint8 moduleId)
static void LogUser_DumpByInd(int argc, char**argv)
{
	int ind = 0;
	int count;
	int moduleId;
	uint8 temp = g_pDbgInfo->pmsAddrForPrintf;

	sscanf(&(*argv[1]), "%d", &ind);
	sscanf(&(*argv[2]), "%d", &count);
	sscanf(&(*argv[3]), "%d", &moduleId);
	g_pDbgInfo->pmsAddrForPrintf = moduleId;
	Record_DumpByInd(&g_LogRecord, ind, count, moduleId);
	g_pDbgInfo->pmsAddrForPrintf = temp;
}
MSH_CMD_EXPORT(LogUser_DumpByInd, LogUser_DumpByInd);

void LogUser_Dump(LogItem* pItem, const char* head, const char* tail)
{
	static char logTypeBuf[LOG_BUF_TYPE_SIZE];
	static char moduleIdBuf[LOG_BUF_MODULEID_ID_SIZE];
	static char EventIdBuf[LOG_BUF_EVENT_ID_SIZE];
	static char EventValueBuf[LOG_BUF_EVENT_VALUE_SIZE];
	if (head == Null) head = "";
	if (tail == Null) tail = "";
	memset(logTypeBuf, 0, sizeof(logTypeBuf));
	memset(moduleIdBuf, 0, sizeof(moduleIdBuf));
	memset(EventIdBuf, 0, sizeof(EventIdBuf));
	memset(EventValueBuf, 0, sizeof(EventValueBuf));
	Printf("%s", head);
	Printf("%s:", DateTime_FmtStr(pItem->dateTime));
	Printf("%s %s.%s=%s%s\n"
		, LogType_ToStr (pItem, logTypeBuf, sizeof(logTypeBuf))
		, ModuleId_ToStr(pItem, moduleIdBuf, sizeof(moduleIdBuf))
//		, CatId_ToStr(pItem, catIdBuf, sizeof(catIdBuf))
//		, SubId_ToStr(pItem, subIdBuf, sizeof(subIdBuf))
		, EventId_ToStr (pItem, EventIdBuf, sizeof(EventIdBuf))
		, LogVal_ToStr  (pItem, EventValueBuf, sizeof(EventValueBuf))
		, tail
	);
}

void LogUser_Write(uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val)
{
	LogItem item;
	LogItem* p = g_pLogItem;
	
	memset(&item,0,sizeof(LogItem));
	if (g_pLogItem == Null)	//g_pLogItem 没被初始化
	{
		p = &item;
	}

	Log_Init(p, moduleId, catId, subId, logType, eventId, val);

	if (g_dwDebugLevel & DL_LOG && LogUser_IsPrintf(p))
	{
		Printf("%04d:", g_LogRecord.itemCount);
		LogUser_Dump(p, Null, Null);
	}

	if (g_pLogItem)
	{
		Record_Write(&g_LogRecord, p);
	}

}

Bool Log_VerifyItem(LogItem* pItem, uint32 param)
{
	return (LOG_DATA_VER == pItem->version);	//校验版本号
}

void LogUser_Init(void)
{
	static LogItem	g_LogItem;
	
	g_LogItem.version = LOG_VERSION;
	g_LogItem.subId = 0;
	//初始化日志记录模块
	g_pLogItem = &g_LogItem;
	Record_Init(&g_LogRecord,
				g_pLogItem,
				(void*)g_pNvdsMap->logBuf,
				sizeof(LogItem),
				MAX_LOG_SECTOR,
				(RecordVerifyItemFn)Log_VerifyItem);
}
