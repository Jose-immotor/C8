#ifndef  _SYS_LOG_H_
#define  _SYS_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus
#include "Log.h"

#define SYS_LOG_EVT_COUNT 6

	//ϵͳ���ID����
	typedef enum _SysCatID
	{
		//system event
		SYS_CATID_COMMON = 1,	//ͨ�����ID��SubID = 0.
		SYS_CATID_IO,			//IO���ID��SubID = IO_ID
		SYS_CATID_ADC,			//ADC���ID��SubID = ADC_ID
		SYS_CATID_SM,			//ϵͳ��������ID, SubID = 0
	}SysCatID;

	//ϵͳ�����SubID���壬����Ϊ0������Ҫ���ⶨ��

	typedef enum _SysEvtID
	{
		SysEvtID_McuReset,		//MCU��λ��      ��ʽ (SYS_CATID_COMMON, 0, CCU_ET_MCU_RESET, reason)
		SysEvtID_HighTempAlarm,	//ϵͳ���¸澯�� ��ʽ (SYS_CATID_COMMON, 0, SysEvtID_HighTempAlarm, ntc, alarmBit)
		SysEvtID_BATIN,			//��ز��룬     ��ʽ (SYS_CATID_COMMON, 0, SysEvtID_BATIN, soc)
		SysEvtID_BATOUT,		//��ذγ���     ��ʽ (SYS_CATID_COMMON, 0, SysEvtID_BATOUT, 0)
		SysEvtID_SysSleep,		//���ߣ�     	 ��ʽ (SYS_CATID_COMMON, 0, SysEvtID_SysSleep, 0)
		SysEvtID_WakeUp,		//���ѣ�     	 ��ʽ (SYS_CATID_COMMON, 0, SysEvtID_SysSleep, 0)
		
	}SysEvtID;

	extern const LogEvt g_SysLogEvts[];

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  


