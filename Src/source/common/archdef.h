#ifndef __ARCH_CFG_H_
#define __ARCH_CFG_H_

#ifdef __cplusplus
extern "C"{
#endif


#define MAX_MSG_COUNT		60	//Message queue deepth
#define MAX_TIMER_COUNT		4
#define MS_STICKS 			1	//Hot many ticks per ms, it is for timer
#define TIMER_TASK_INTERVAL 1
#define MAX_TEMP_BUF_SIZE 	512


extern volatile unsigned int sim_ms_tick;
#define GET_TICKS( ) sim_ms_tick


#include "typedef.h"	
#include "debug.h"	
#include "string.h"
#include <stdio.h>
#include <stdlib.h>


typedef int (*EventProcFun)(void* pObj, uint8 eventId, void* pData, int len);
typedef struct _FsmMap
{
	uint8 			m_State;
	EventProcFun	Proc;
}FsmMap;


typedef struct _ArchDef
{
	uint8 id;
}ArchDef;

typedef void (*TaskFun)(void* pArg);
typedef void (*voidObjFun)(void* pObj);
typedef int (*ObjFun)(void* pObj);
typedef int (*ObjIsFun)(void* pObj);
typedef int (*ObjSetFun)(void* pObj, uint32 value);
typedef Bool (*EnableFun)(void* pObj, Bool isEnable);

extern Bool Arch_PostMsg(void* pObj, uint32 msgID, uint32 param1, uint32 param2);
extern Bool Arch_PostMsg(void* pObj, uint32 msgID, uint32 param1, uint32 param2);
extern uint8 g_TempBuff[MAX_TEMP_BUF_SIZE];
int Osa_Init(void);

#define READ_REG32(reg) (uint32)(*(vuint32*)(reg))
#define WRITE_REG32(reg, vvalue) (*(vuint32*)(reg)) = vvalue
#define MODIFY_REG32(reg, clear_mask, set_mask) ((*(vuint32*)(reg)) = ((*(vuint32*)(reg)) &~ clear_mask) | set_mask)
#define MIN(v1, v2) ((v1) > (v2) ? (v2) : (v1))
#define MAX(v1, v2) ((v1) < (v2) ? (v2) : (v1))
#define GET_ELEMENT_COUNT(array) (sizeof(array)/sizeof(array[1]))
#define Assert RT_ASSERT
#ifdef __cplusplus
}
#endif

#endif




