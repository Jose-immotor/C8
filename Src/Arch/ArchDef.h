#ifndef __ARCH_CFG_H_
#define __ARCH_CFG_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
	
	#include <windows.h>
	#include <stdio.h>

	#define SLEEP 		Sleep
	#define GET_TICKS() system_ms_tick	//GetTickCount()
	#define UDELAY(...)
	
	extern CRITICAL_SECTION g_CriticalSection;
	//#define Printf printf
#else
	#define MDELAY(mS) TIMER_Delay(TIMER0, (mS)*1000)
	#define UDELAY(uS) TIMER_Delay(TIMER0, (uS))
	
	#define GET_TICKS( ) system_ms_tick
#endif

#include "typedef.h"	
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Dump.h"
#include "_Macro.h"
#include "Dbg.h"

#define CFG_LOG
//#define XDEBUG

	//ms ticks of system, 
	extern volatile unsigned int system_ms_tick;

#ifdef __cplusplus
}
#endif

#endif




