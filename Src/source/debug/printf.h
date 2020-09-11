#ifndef  _PRINTF_H_
#define  _PRINTF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"


#define PRINTF_BUF_SIZE 1024

#define Printf rt_kprintf//_Printf

//#define _PrintfLevel rt_kprintf//_Printf
//	if( 0 == (level & g_dwDebugLevel)) return;
//	rt_kprintf(lpszFormat,...);

void _PrintfLevel(uint32 level, const char* lpszFormat, ...);
int _Printf( const char* lpszFormat, ...);
int Printf_None(const char* lpszFormat, ...);

int vsnprintfEx(char* buffer, int bufLen, const char* lpszFormat, va_list ptr);

#ifdef __cplusplus
}
#endif

#endif

