
#include "Common.h"
#include "LogUser.h"
#include "MeterLog.h"

const LogEventMap g_MeterStrLogEvent[] =
{
	 {0 ,0 ,ET_METER_RESET				, "MCU_RESET"      , 0},
	 {0 ,0 ,ET_METER_OP_STATE_CHANGED   , "OPER_STATE"     , OPERATION_STR },
};

const char* Meter_EventIdToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_MeterStrLogEvent, GET_ELEMENT_COUNT(g_MeterStrLogEvent), pItem);
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

const char* Meter_EventValueToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_MeterStrLogEvent, GET_ELEMENT_COUNT(g_MeterStrLogEvent), pItem);
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
