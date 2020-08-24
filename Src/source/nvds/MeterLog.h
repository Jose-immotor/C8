
#ifndef _METER_LOG_EVENT_H_
#define _METER_LOG_EVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Log.h"


	typedef enum _MeterLogEventID
	{
		//system event
		ET_METER_RESET = 1,
		ET_METER_OP_STATE_CHANGED = 0x10,		//电表操作状态改变，格式 (ET_METER_OP_STATE_CHANGED, newState)
	}MeterLogEventID;

	const char* Meter_EventIdToStr(const LogItem* pItem, char* buf, int bufSize);
	const char* Meter_EventValueToStr(const LogItem* pItem, char* buf, int bufSize);

#ifdef __cplusplus
}
#endif

#endif
