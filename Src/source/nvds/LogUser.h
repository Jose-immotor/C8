#ifndef _LOG_USER_H_
#define _LOG_USER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Log.h"
#define OPERATION_STR "%0-7B{0-Init|4-Stopped|5-Operation|127-PreOperation}"
//如果修改过LOG结构的定义，之前保存在FLASH的LOG值作废，需要修改LOG_VERSION和之前不同
#define LOG_VERSION 0

#define	ET_MODULEID_UNDEFIND 0 //没定义的CAT ID
#define	ET_CATID_UNDEFIND 0	//没定义的CAT ID
#define	ET_SUBID_UNDEFIND 0	//没定义的SUB ID

#define CONFIG_LOG
#ifdef CONFIG_LOG
#define LOG_TRACE1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; LogUser_Write(moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; LogUser_Write(moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L	 ; LogUser_Write(moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0); LogUser_Write(moduleId, catId, subId, LT_TRACE , eventId, _val);}

#define LOG_WARNING1(moduleId, catId, subId, eventId, __val) 		{ uint32 _val = (__val)					    ; LogUser_Write(moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		    ; LogUser_Write(moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L   ; LogUser_Write(moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0) ; LogUser_Write(moduleId, catId, subId, LT_WARNING , eventId, _val);}

#define LOG_ERROR1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; LogUser_Write(moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; LogUser_Write(moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR3(moduleId, catId, subId, eventId, asU16H, asU16L)	{ uint32 _val = (asU16H << 16) | asU16L	 ; LogUser_Write(moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR4(moduleId, catId, subId, eventId, b3, b2, b1, b0)	{ uint32 _val = AS_UINT32(b3, b2, b1, b0); LogUser_Write(moduleId, catId, subId, LT_ERROR , eventId, _val);}
#else
#define LOG_TRACE1(...) 			
#define LOG_TRACE2(...)		
#define LOG_TRACE3(...)
#define LOG_TRACE4(...)

#define LOG_WARNING1(...)	
#define LOG_WARNING2(...)		
#define LOG_WARNING3(...)
#define LOG_WARNING4(...)

#define LOG_ERROR1(...)			
#define LOG_ERROR2(...)		
#define LOG_ERROR3(...)	
#define LOG_ERROR4(...)
#endif

void LogUser_Dump(LogItem* pItem, const char* head, const char* tail);
void LogUser_Init(void);
void LogUser_Write(uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val);
//void LogUser_DumpByCount(int count, uint8 moduleId);
//void LogUser_DumpByInd(int ind, int count, uint8 moduleId);
void LogUser_Tester();
const LogEventMap* Log_GetLogEventMap(const LogEventMap* pLogEventMap, int count, const LogItem* pItem);
const char* EventValue_ToStr(const LogItem* pItem, const char* fmt, char* buf, int bufSize);
Bool LogUser_IsPrintf(const LogItem* p);

#ifdef __cplusplus
}
#endif

#endif
