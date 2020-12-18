
#ifndef __DRV_PM_H__
#define __DRV_PM_H__

#include "typedef.h"

typedef enum _WakeupType
{
	 WAKEUP_POWER = 0x0001
	,WAKEUP_GYRO = 0x0002
	,WAKEUP_RTC = 0x0004
	,WAKEUP_GPRS_INSERT = 0x0008
	,WAKEUP_CAN = 0x0010
	,WAKEUP_BAT_INSERT = 0x0020
	,WAKEUP_TAKE_APART = 0x0040
	,WAKEUP_OVER_TEMP = 0x0080
	,WAKEUP_CABIN_LOCK = 0x0100
	,WAKEUP_MAX = 0xFFFF
}WakeupType;

extern volatile bool g_isPowerDown;//休眠标志，False-没有休眠，True-休眠


void SetWakeUpType(WakeupType type);
void Boot(void);
void Mcu_PowerDown();

#endif //__DRV_PM_H__
