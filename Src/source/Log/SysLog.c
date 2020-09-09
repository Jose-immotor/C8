#include "Common.h"
#include "SysLog.h"

//ϵͳ��־�¼�
const LogEvt g_SysLogEvts[SYS_LOG_EVT_COUNT] =
{
	 {SYS_CATID_COMMON ,0 ,SysEvtID_McuReset, "MCU-RESET"	 , "Reason:%0-7B"},
	 {SYS_CATID_COMMON ,0 ,SysEvtID_HighTempAlarm, "HighTempAlarm"  , "Ntc:%16-31B{3-LcdNtc|4-ChgrNtc|5-PmsOnBoard|6-PmsOutside}; AlarmBit[%15-0BX]"},
	 {SYS_CATID_COMMON ,0 ,SysEvtID_BATIN,  "BAT-IN"	 , "SOC(0.1):%0-16B"},
	 {SYS_CATID_COMMON ,0 ,SysEvtID_BATOUT, "BAT-OUT"	 },
};

//Sample�� add a log
//LOG_TRACE1(SYS_CATID_COMMON, 0, SysEvtID_McuReset, reason);
//LOG_TRACE3(SYS_CATID_COMMON, 0, SysEvtID_HighTempAlarm, ntc, alarmBit);
