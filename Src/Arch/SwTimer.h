#ifndef __SWTIMER_H_
#define __SWTIMER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

typedef struct _SwTimer
{
	Bool	m_isStart;
	uint32_t 	m_InitTicks;
	uint32_t 	m_TimeOutTicks;
	uint32_t 	m_Id;
}SwTimer;

void SwTimer_Init(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId);
void SwTimer_ReStart(SwTimer* pTimer);
void SwTimer_Start(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId);
void SwTimer_Stop(SwTimer* pTimer);
Bool SwTimer_isTimerOut(SwTimer* pTimer);
Bool SwTimer_isTimerOutEx(uint32_t initTicks, uint32_t timeOutTicks);
Bool SwTimer_IsStart(SwTimer* pTimer);
Bool SwTimer_isTimerOut_onId(SwTimer* pTimer, uint32_t timeId);


#ifdef __cplusplus
}
#endif

#endif

