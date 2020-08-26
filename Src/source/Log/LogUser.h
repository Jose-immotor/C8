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

	void LogUser_init();


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  


