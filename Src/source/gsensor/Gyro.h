
#ifndef __GYRO_H_
#define __GYRO_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "common.h"

typedef struct _Gyro
{
	Bool 	m_isGyroOk;
	Bool 	m_isGyroWakeUp;
	Bool 	m_isPowerOn;
	
	int  	m_TriggerCount;
	SwTimer m_Timer;
}Gyro;

extern Gyro g_Gyro;

void Gyro_Sleep(void);
void Gyro_Start(void);
void Gyro_Init(void);
void Gyro_Run(void);
void Gyro_Reset(void);
Bool Gyro_isIdle(uint32 second);
void Gyro_Stop(void);


#ifdef __cplusplus
}
#endif

#endif

