#include "Common.h"
#include "Log.h"
#include "CcuLog.h"
#include "LogUser.h"
//#include "DriverIo.h"

const LogEventMap g_CcuStrLogEvent[] =
{
	 {CCU_CATID_COMMON ,0 ,CCU_ET_MCU_RESET       , "MCU_RESET"      , "%0-7B{1-SmCmd|2-UpgFw|3-PmsCanErr}"},
	 {CCU_CATID_COMMON ,0 ,CCU_ET_PMS_POWEROFF    , "PmsPowerOff"    , "Reason:%0-7B{1-ShelfWaterIn|2-HighTemp}"},
	 {CCU_CATID_COMMON ,0 ,CCU_ET_HIGH_TEMP_ALARM , "AlarmByHighTemp", "Ntc:%16-31B{3-LcdNtc|4-ChgrNtc|5-PmsOnBoard|6-PmsOutside}; AlarmBit[%15-0BX]"},
	 {CCU_CATID_COMMON ,0 ,CCU_ET_FSM_SWITCH      , "OPER_STATE"     , OPERATION_STR },
	 {CCU_CATID_COMMON ,0 ,CCU_ET_PUMP_ON		  , "PumpOdOn"       , "FireAlarm[0-15]:16B%17B%18B%19B %20B%21B%22B%23B %24B%25B%26B%27B %28B%29B%30B%31B; FireAlarm[0-15]:%0-15BX" },

	 {CCU_CATID_SM     ,0 ,ET_SM_CONNECTION_CHANGED , "SmConnection", "%0-7B{0-DISCONNECT|1-CONNECT}"},
	 {CCU_CATID_SM     ,0 ,ET_SM_RESET , "SmReset", "%0-7B{1-ResetCmd|2-CommErr6Min|3-CommErr12Min}"},

//	 {CCU_CATID_IO, CTRL_PUMP	  , CCU_ET_IO_SET	, "PumpIOPowerON"},
//	 {CCU_CATID_IO, CTRL_12VBP_ON , CCU_ET_IO_SET   , "12VBP_ON", "0-7B%{0-OFF|1-ON}"},

//	 {CCU_CATID_IO ,DET_12V_1    ,CCU_ET_IO_DET_FAULT, "12V_1_ERROR" , "Value:%0-7B"},
//	 {CCU_CATID_IO ,DET_12V_2    ,CCU_ET_IO_DET_FAULT, "12V_2_ERROR" , "Value:%0-7B"},
//	 {CCU_CATID_IO ,DET_12V_3    ,CCU_ET_IO_DET_FAULT, "12V_3_ERROR" , "Value:%0-7B"},
//	 {CCU_CATID_IO ,DET_12V_4    ,CCU_ET_IO_DET_FAULT, "12V_4_ERROR" , "Value:%0-7B"},
//	 {CCU_CATID_IO ,DET_12V_BP   ,CCU_ET_IO_DET_FAULT, "12V_BP_ERROR", "Value:%0-7B"},
//	 {CCU_CATID_IO ,DET_PUMP_OFF ,CCU_ET_IO_DET_FAULT, "PUMP_OFF"    , "Value:%0-7B"},
};

const char* Ccu_EventIdToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_CcuStrLogEvent, GET_ELEMENT_COUNT(g_CcuStrLogEvent), pItem);
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

const char* Ccu_EventValueToStr(const LogItem* pItem, char* buf, int bufSize)
{
	const LogEventMap* p = Log_GetLogEventMap(g_CcuStrLogEvent, GET_ELEMENT_COUNT(g_CcuStrLogEvent), pItem);
	if (p)
	{
//		if (p->catId == CCU_CATID_IO)
//		{
//			DrvIo* pIo = IO_Get((IO_ID)p->subId);
//			if (pIo)
//			{
//				sprintf(buf, "%s-%s: %d\n", PortPinToPxx(pIo->periph, pIo->pin), pIo->desc, pIo->value);
//			}
//			else
//			{
//				sprintf(buf, "Ccu_EventValueToStr Error: catId=%d,subId=%d, evetId=%d:\n", p->catId, p->subId, p->eventId);
//			}
//		}
//		else
		 return EventValue_ToStr(pItem, p->pEventValueStrFmt, buf, bufSize);
	}
	else
	{
		sprintf(buf, "0x%08X", pItem->asUint32);
	}
	return buf;
}
