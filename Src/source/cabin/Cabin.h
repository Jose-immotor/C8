#ifndef __CABIN_LOCK_H_
#define __CABIN_LOCK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "drv_gpio.h"
// *��״̬��0-���������ϣ�1-������
//����������������-�͵�ƽ��û����-�ߵ�ƽ
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


