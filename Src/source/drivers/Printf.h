#ifndef  _PRINTF_H_
#define  _PRINTF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "ArchDef.h"
#include <stdarg.h>
  
#define PRINTF_BUF_SIZE 256

#define Printf _Printf

void _PrintfLevel(uint32 level, const char* lpszFormat, ...);
int _Printf( const char* lpszFormat, ...);
int Printf_None(const char* lpszFormat, ...);

int vsnprintfEx(char* buffer, int bufLen, const char* lpszFormat, va_list ptr);

#ifdef __cplusplus
}
#endif

#endif

