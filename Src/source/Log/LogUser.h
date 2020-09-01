#ifndef  _LOG_USER_H_
#define  _LOG_USER_H_

#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus

#include "Log.h"
	
#ifdef CFG_LOG

#define LOG_TRACE1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}

#define LOG_WARNING1(moduleId, catId, subId, eventId, __val) 		{ uint32 _val = (__val)					    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L   ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0) ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}

#define LOG_ERROR1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR3(moduleId, catId, subId, eventId, asU16H, asU16L)	{ uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR4(moduleId, catId, subId, eventId, b3, b2, b1, b0)	{ uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}

	typedef enum _LogModuleID
	{
		LogModuleID_SYS,
		LogModuleID_2,
	}LogModuleID;

	void LogUser_init();

	extern LogMgr* g_plogMgr;
	
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

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  


