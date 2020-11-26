#ifndef  _SYS_LOG_H_
#define  _SYS_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus
#include "Log.h"

#define SYS_LOG_EVT_COUNT 12

	//系统类别ID定义
	typedef enum _SysCatID
	{
		//system event
		SYS_CATID_COMMON = 1,	//通用类别ID，SubID = 0.
		SYS_CATID_IO,			//IO类别ID，SubID = IO_ID
		SYS_CATID_ADC,			//ADC类别ID，SubID = ADC_ID
		SYS_CATID_SM,			//系统管理子类ID, SubID = 0
	}SysCatID;

	//系统子类别SubID定义，保持为0，不需要另外定义

	typedef enum _SysEvtID
	{
		SysEvtID_McuReset,		//MCU复位，      格式 (SYS_CATID_COMMON, 0, CCU_ET_MCU_RESET, reason)
		SysEvtID_HighTempAlarm,	//系统高温告警， 格式 (SYS_CATID_COMMON, 0, SysEvtID_HighTempAlarm, ntc, alarmBit)
		SysEvtID_BATIN,			//电池插入，     格式 (SYS_CATID_COMMON, 0, SysEvtID_BATIN, soc)
		SysEvtID_BATOUT,		//电池拔出，     格式 (SYS_CATID_COMMON, 0, SysEvtID_BATOUT, 0)
		SysEvtID_SysSleep,		//休眠，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_SysSleep, 0)
		SysEvtID_WakeUp,		//唤醒，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_WakeUp, WakeUpType)
		SysEvtID_TakeApart,		//拆开，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_TakeApart, 0)
		SysEvtID_OverTemp,		//过温，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_OverTemp, 0)
		SysEvtID_SetAccOn,		//点火，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_SetAccOn, 0)
		SysEvtID_SetAccOff,		//熄火，     	 格式 (SYS_CATID_COMMON, 0, SysEvtID_SetAccOff, 0)
		SysEvtID_18650LowPower,	//18650电压低，  格式 (SYS_CATID_COMMON, 0, SysEvtID_18650LowPower, 电压)
		SysEvtID_18650HigPower,	//18650电压高，  格式 (SYS_CATID_COMMON, 0, SysEvtID_18650HigPower, 电压)
		
	}SysEvtID;

	extern const LogEvt g_SysLogEvts[];

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  


