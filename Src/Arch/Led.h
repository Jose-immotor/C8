
#ifndef __LED__H_
#define __LED__H_
/*
1. ÿ��LED֧������LED����: ON, OFF, SHINE��
2. ֧�ֶ���һ��LED��ÿ��������LED.
3. ֧��LED�������һ��������������LED���
4. ֧��LedsMode��������������һ��LedsMode����.
5. ֧��LedsMode�Ŷӣ�
	a. ���������ֻ��һ��LedsMode��֧������ִ�л��߽�ִ��һ��
	b. ����������ж��LedsMode��ÿ��LedsMode����ִ����һ�����ڲŻ�ִ����һ��LedsMode��
 
*/
#ifdef __cplusplus
extern "C"{
#endif

#include "common.h"
#include "Queue.h"

#define MAX_LED_COUNT 		1
#define INVALID_TIME_VALUE 	(0xFFFF)
#define TIME_ONCE 	 		(0xFFFE)	//��ʾ��ǰ״ֻ̬����һ��Ȼ�����
#define INVALID_MODE_ID 	(0xFF)		//��ʾ��Ч��ModeID

typedef enum _LedStatus
{
	 LED_STATUS_OFF
	,LED_STATUS_ON
	,LED_STATUS_SHINE
}LedStatus;

//LED command
typedef struct _LedState
{
	uint8  m_LedId:4;
	uint8  m_State:4;
	
	uint16 m_onMs;
	uint16 m_offMs;
}LedState;

struct _Led;
typedef void (*LedSetPowerFn)(struct _Led* pLed, Bool isOn);
typedef struct _Led
{
	uint8 	m_LedID;
	uint8 	m_State;

	uint32 	m_StartTimeMs;
	uint16 	m_TimeOutMs;
	uint8 	m_isDelay;

	uint8  m_PeindingMaxCount;
	const LedState* m_pPendingWorkMode;
	
	uint8  m_MaxCount;
	uint8  m_count;
	const LedState* m_pWorkMode;

	LedSetPowerFn SetPower;
}Led;
void Led_Init(Led* pLed, uint8 ledID, LedSetPowerFn LedSetPower);
void Led_SetWorkModeEx(Led* pLed, const LedState* pLedState, uint8 count);
void Led_Run(Led* pLed);
void Led_Reset(Led* pLed);


//LED group cmd
typedef struct _LedsCmdItem
{
	uint16 	m_TimeOutMs;
	LedState	m_ledCmd[MAX_LED_COUNT];
}LedsCmdItem;

//LEDs mode
typedef struct _LedsMode
{
	uint8		m_isStop:1;
	uint8		m_Reserved:7;
	
	uint8		m_ModeId;
	uint8		m_CmdCount;
	const LedsCmdItem*	m_pLedsCmdItemArray;
}LedsMode;

//LED group mgr
#define MAX_LED_CMD_COUNT 4
typedef struct _LedGroup
{
	uint8 	m_State;

	uint8	m_LedCount;
	Led*	m_LedArray;

	LedsMode* 	m_pLedsMode;		//��ǰ����ִ�е�LedsMode

	uint8		m_isFullCycle:1;
	uint8		m_Reserved:1;			//The index of LedStates in LedsMode
	uint8		m_CmdIndex:6;		//Max is 64 
	
	uint32		m_StartTimeMs;
	
	Queue		m_Queue;
	LedsMode	m_QueueBuff[MAX_LED_CMD_COUNT];
}LedGroup;

void LedGroup_Init(LedGroup* pLedGroup, Led* pLedArray, uint8 ledCount);
void LedGroup_Run(LedGroup* pLedGroup);
Bool LedGroup_SetWorkMode(LedGroup* pLedGroup, uint8 modeId, const LedsCmdItem* m_pLedsCmdItemArray, int nCount);
void LedGroup_RemoveAll(LedGroup* pLedGroup);
Bool LedGroup_IsEmpty(LedGroup* pLedGroup);
uint8 LedGroup_GetModeId(LedGroup* pLedGroup);
Bool LedGroup_Stop(LedGroup* pLedGroup, uint8 modeId);

#ifdef __cplusplus
}
#endif

#endif



