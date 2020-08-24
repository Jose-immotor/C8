#include "Common.h"
#include "LogUser.h"
#include "ShelfLockLog.h"

const LogEventMap g_ShelfLockStrLogEvent[] =
{
	 {0 ,0 ,LOCK_ET_RESET				, "MCU_RESET"      , 0},
	 {0 ,0 ,LOCK_ET_OP_STATE_CHANGED   , "OPER_STATE"     , OPERATION_STR },
};

const char* ShelfLock_EventIdToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_ShelfLockStrLogEvent, GET_ELEMENT_COUNT(g_ShelfLockStrLogEvent), pItem);
	if (p)
	{
		strcpy(buf, p->pEventIdStr);
	}
	else
	{
		sprintf(buf, "EventID[%02X]", pItem->eventId);
	}
	return buf;
}

const char* ShelfLock_EventValueToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_ShelfLockStrLogEvent, GET_ELEMENT_COUNT(g_ShelfLockStrLogEvent), pItem);
	if (p)
	{
		EventValue_ToStr(pItem, p->pEventValueStrFmt, buf, bufSize);
	}
	else
	{
		sprintf(buf, "0x%08X", pItem->asUint32);
	}
	return buf;
}
