
#ifndef _SHELF_LOCK_LOG_EVENT_H_
#define _SHELF_LOCK_LOG_EVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Log.h"

	typedef enum _LockEventID
	{
		//system event
		LOCK_ET_RESET = 1,
		LOCK_ET_STATE_CHANGED,		//后门锁状态改变，格式 (LOCK_ET_STATE_CHANGED, newState)
		LOCK_ET_OP_STATE_CHANGED,	//后门锁操作状态改变，格式 (LOCK_ET_OP_STATE_CHANGED, newState)
	}LockEventID;


	const char* ShelfLock_EventIdToStr(const LogItem* pItem, char* buf, int bufSize);
	const char* ShelfLock_EventValueToStr(const LogItem* pItem, char* buf, int bufSize);

#ifdef __cplusplus
}
#endif

#endif
