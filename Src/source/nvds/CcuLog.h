
#ifndef _CCU_LOG_EVENT_H_
#define _CCU_LOG_EVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Log.h"

//CCU catlog ID, CCU类别ID
typedef enum _CcuCatID
{
	//system event
	CCU_CATID_COMMON = 1,	//通用类别ID，SubID = 0.
	CCU_CATID_IO,			//IO类别ID，SubID = IO_ID
	CCU_CATID_ADC,			//ADC类别ID，SubID = ADC_ID
	CCU_CATID_SM,			//系统管理子类ID, SubID = 0
}CcuCatID;

typedef enum _CCU_EventID
{
	//CCU_CATID_COMMON event
	CCU_ET_MCU_RESET = 1,		//MCU复位，格式(CCU_CATID_COMMON, 0, CCU_ET_MCU_RESET, reason)
	CCU_ET_PMS_POWEROFF,		//PMS断电，(CCU_CATID_COMMON, 0, CCU_ET_PMS_POWEROFF, CcuResetReason)
	CCU_ET_PUMP_ON,				//水泵标志启动， 格式 (CCU_CATID_COMMON, 0, CCU_ET_PUMP_ON, pmsFireAlarmFlag, pmsFireAlarmFlag)

	CCU_ET_FSM_SWITCH = 0x10,	//FSM状态切换，格式 (CCU_CATID_COMMON, CCU_CATID_COMMON, CCU_ET_FSM_SWITCH, newFsmState)
	CCU_ET_HIGH_TEMP_ALARM,		//CCU高温告警， 格式 (CCU_CATID_COMMON, 0, CCU_ET_HIGH_TEMP_ALARM, ntc, alarmBit)

	//CCU_CATID_SM event
	ET_SM_CONNECTION_CHANGED = 0x20,	//SM连接改变命令，格式 (ET_SM_CONNECTION_CHANGED, cnt/discnt)
	ET_SM_RESET,						//SM控制命令，格式 (ET_SM_CMD_CHARGE, cmd, operationState)
	//ET_SM_CMD_LOCK,

	////CCU_CATID_IO event
	CCU_ET_IO_CHANGED = 0x30,		//IO状态改变，格式(CCU_CATID_IO, IO_ID, CCU_ET_IO_CHANGED, newValue)
	CCU_ET_IO_SET,					//IO状态设置，格式(CCU_CATID_IO, IO_ID, CCU_ET_IO_SET    , newValue)
	CCU_ET_IO_DET_FAULT,			//IO检测到故障，格式(CCU_CATID_IO, IO_ID, CCU_ET_IO_DET_FAULT, ioValue)

}CCU_EventID;

const char* Ccu_EventIdToStr(const LogItem* pItem, char* buf, int bufSize);
const char* Ccu_EventValueToStr(const LogItem* pItem, char* buf, int bufSize);

#ifdef __cplusplus
}
#endif

#endif
