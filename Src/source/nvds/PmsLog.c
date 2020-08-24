#include "Common.h"
#include "PmsLog.h"
#include "Pms.h"
#include "CcuDef.h"
#include "LogUser.h"

const LogEventMap g_pPmsStrLogEvent[] =
{
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_RESET , "PMS_RESET", 0},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_OP_STATE_CHANGE , "OP_STATE_CHANGE", OPERATION_STR},

	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_BAT_PRESENT_CHANGED , "BAT_PRESENT_CHANGED", STR_PMS_STATE1("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_DOOR_STATE_CHANGED  , "DOOR_STATE_CHANGED" , STR_PMS_STATE1("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_BAT_LOCK_CHANGED    , "BAT_LOCK_CHANGED"   , STR_PMS_STATE1("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_FIRE_ALARM		   , "FIRE_ALARM"		 , STR_PMS_STATE1("; ")},

	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CHG_ENABLE_CHANGED  , "CHG_ENABLE"  , STR_PMS_STATE2("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_STATE2	, "PMS_STATE2"  , STR_PMS_STATE2("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_STATE3	, "PMS_STATE3"  , STR_PMS_STATE3("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_STATE4	, "PMS_STATE4"  , STR_PMS_STATE4("; ")},

	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CAP_TEST_RESULT, "CAPA_TEST_ERR_CODE", STR_CAPA_TEST_RESULT("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CAP_TEST_STATE , "CAPA_TEST_STATE" , STR_CAPA_TEST_STATE()},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CAP_TEST_VALUE , "CAPA_TEST_VALUE" , "Capacity:%0-15B, SOC:%16-31B"},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CAP_TEST_TIME  , "CAPA_TEST_COST_SECONDS" , "Seconds:%0-15B"},

	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CMD_STATE, "PMS_CMDSTATE", STR_PMS_CMD("; ")},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_FIRE		, "PMS_FIRE"	, "SOC[%8-15B],%0-7B{0:FIRE OFF|1:FIRE ON}"},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_DISCHG	, "PMS_DISCHG"	, "SOC[%24-31B],%16-23B{1-DischgEn|0-DischgDisable},%8-15B{0-Start|1-End},%0-7B{0-OK|1-Err}"},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_PMS_CHG		, "PMS_CHG"		, "SOC[%8-15B],IS_ON[%0-7B]"},
	 {PMS_CATID_SYS ,PMS_SBUID_COMMON ,ET_SOC_CHANGED	, "SOC_CHANGED"	, "SOC:[%0-7B]"},
};

const char* Pms_ModuleValueToStr(const LogItem* pItem, char* buf, int bufSize)
{
//	MasterNode* p = Master_FindMasterNode(pItem->moduleId);
//	if (p && p->addr >= PMS_ADDR_0 && p->addr <= PMS_ADDR_12)
//	{
//		sprintf(buf, "%s[%02X]", p->name, pItem->moduleId);
//	}
//	else
//	{
//		sprintf(buf, "OBJ[%02X]", pItem->moduleId);
//	}	
	return buf;
}

const char* Pms_EventIdToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_pPmsStrLogEvent, GET_ELEMENT_COUNT(g_pPmsStrLogEvent), pItem);
	if (p && p->pEventIdStr)
	{
		return p->pEventIdStr;
	}

	sprintf(buf, "%02X", pItem->eventId);
	return buf;
}

const char* Pms_EventValueToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_pPmsStrLogEvent, GET_ELEMENT_COUNT(g_pPmsStrLogEvent), pItem);
	if (p && p->pEventValueStrFmt)
	{
		return EventValue_ToStr(pItem, p->pEventValueStrFmt, buf, bufSize);
	}
	else
	{
		return EventValue_ToStr(pItem, Null, buf, bufSize);
	}
}

void Pms_Log(const Pms* pPms, uint8 event, uint32 value)
{

}
