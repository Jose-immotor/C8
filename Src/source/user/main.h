

#ifndef MAIN_H
#define MAIN_H

#include "typedef.h"
#include "gd32f403.h"
#include "common.h"
#include <stdbool.h>

extern uint32 g_ResetMs;
extern volatile bool g_isPowerDown;//���߱�־��False-û�����ߣ�True-����
extern WakeupType g_WakeupType;

/* led spark function */
void led_spark(void);
void DelayPowerOff(uint32 ms);
void ResetStop();
void SetWakeUpType(WakeupType type);
void Boot(Bool isFromAppRom);
#endif
