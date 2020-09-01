#ifndef  _LOG_USER_H_
#define  _LOG_USER_H_

#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus

	typedef enum _LogModuleID
	{
		LogModuleID_SYS,
		LogModuleID_2,
	}LogModuleID;

//	#define LOG_TRACE1(moduleId, catId, subId, eventId, __val) \
//		{uint32 _val = (__val);LogUser_Write(moduleId, catId, subId, LT_TRACE , eventId, _val);}
	
	void LogUser_init();


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  


