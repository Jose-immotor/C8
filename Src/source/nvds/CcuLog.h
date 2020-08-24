
#ifndef _CCU_LOG_EVENT_H_
#define _CCU_LOG_EVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Log.h"

//CCU catlog ID, CCU���ID
typedef enum _CcuCatID
{
	//system event
	CCU_CATID_COMMON = 1,	//ͨ�����ID��SubID = 0.
	CCU_CATID_IO,			//IO���ID��SubID = IO_ID
	CCU_CATID_ADC,			//ADC���ID��SubID = ADC_ID
	CCU_CATID_SM,			//ϵͳ��������ID, SubID = 0
}CcuCatID;

typedef enum _CCU_EventID
{
	//CCU_CATID_COMMON event
	CCU_ET_MCU_RESET = 1,		//MCU��λ����ʽ(CCU_CATID_COMMON, 0, CCU_ET_MCU_RESET, reason)
	CCU_ET_PMS_POWEROFF,		//PMS�ϵ磬(CCU_CATID_COMMON, 0, CCU_ET_PMS_POWEROFF, CcuResetReason)
	CCU_ET_PUMP_ON,				//ˮ�ñ�־������ ��ʽ (CCU_CATID_COMMON, 0, CCU_ET_PUMP_ON, pmsFireAlarmFlag, pmsFireAlarmFlag)

	CCU_ET_FSM_SWITCH = 0x10,	//FSM״̬�л�����ʽ (CCU_CATID_COMMON, CCU_CATID_COMMON, CCU_ET_FSM_SWITCH, newFsmState)
	CCU_ET_HIGH_TEMP_ALARM,		//CCU���¸澯�� ��ʽ (CCU_CATID_COMMON, 0, CCU_ET_HIGH_TEMP_ALARM, ntc, alarmBit)

	//CCU_CATID_SM event
	ET_SM_CONNECTION_CHANGED = 0x20,	//SM���Ӹı������ʽ (ET_SM_CONNECTION_CHANGED, cnt/discnt)
	ET_SM_RESET,						//SM���������ʽ (ET_SM_CMD_CHARGE, cmd, operationState)
	//ET_SM_CMD_LOCK,

	////CCU_CATID_IO event
	CCU_ET_IO_CHANGED = 0x30,		//IO״̬�ı䣬��ʽ(CCU_CATID_IO, IO_ID, CCU_ET_IO_CHANGED, newValue)
	CCU_ET_IO_SET,					//IO״̬���ã���ʽ(CCU_CATID_IO, IO_ID, CCU_ET_IO_SET    , newValue)
	CCU_ET_IO_DET_FAULT,			//IO��⵽���ϣ���ʽ(CCU_CATID_IO, IO_ID, CCU_ET_IO_DET_FAULT, ioValue)

}CCU_EventID;

const char* Ccu_EventIdToStr(const LogItem* pItem, char* buf, int bufSize);
const char* Ccu_EventValueToStr(const LogItem* pItem, char* buf, int bufSize);

#ifdef __cplusplus
}
#endif

#endif
