
#ifndef __DRV_PM_H__
#define __DRV_PM_H__

#include "typedef.h"

typedef enum _WakeupType
{
	 WAKEUP_POWER = 0
	,WAKEUP_GYRO
	,WAKEUP_RTC
	,WAKEUP_GPRS_INSERT
	,WAKEUP_TAKE_APART
	,WAKEUP_MAX
}WakeupType;

extern volatile bool g_isPowerDown;//休眠标志，False-没有休眠，True-休眠


void SetWakeUpType(WakeupType type);
void Boot(void);
void Mcu_PowerDown();

#endif //__DRV_PM_H__
