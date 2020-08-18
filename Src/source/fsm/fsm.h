#ifndef __FSM__H_
#define __FSM__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "common.h"

#define TIME_UPDATE_GPS_TO_SERVER 	(5*60*1000)

typedef enum _FSM_STATE
{
	 FSM_KEY_OFF = 0	//Կ�׿�
	,FSM_KEY_ON		//Կ�׹�
	,FSM_POWERDOWN	//�ػ�
	,FSM_FW_UPGRADE	//�̼�����
	,FSM_KEY_CNT
	,FSM_MAX		//
}FSM_STATE;

void Fsm_Init(void);
void Fsm_Run(void);
void Fsm_Start(void);
void Fsm_Dump(void );
uint8 Fsm_Get(void);
void Fsm_SetMaxGpsCount(int value);
void PostMsg(uint8 msgId);
void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);
void Fsm_SetState(FSM_STATE state);
#ifdef __cplusplus
}
#endif

#endif

