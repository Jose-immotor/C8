#ifndef __CABIN_LOCK_H_
#define __CABIN_LOCK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "drv_gpio.h"
// *锁状态：0-关锁、锁上；1-开锁。
//座舱锁反馈，锁上-低电平。没锁上-高电平
#define CABIN_IS_LOCK		(IO_Read(IO_CABIN_FB) == RESET)

void Cabin_Run(void);
void Cabin_UnLock(void);
void Cabin_Init(void);
//void Cabin_HwInit(void);

extern Bool g_IsCabinLock;
extern Bool g_IsCabinLockFault;

#ifdef __cplusplus
}
#endif

#endif


