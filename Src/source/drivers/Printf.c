
#include "Printf.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
 
extern void _puts (const char *s);

int vsnprintfEx(char* buffer, int bufLen, const char* lpszFormat, va_list ptr)
{
	//if(DL_SHOW_TICKS & g_dwDebugLevel)
	if(0)
	{
		int i = 0;
		int nLen = 0;
		static Bool isNewLine = True;
		
		if(isNewLine)
		{
			if('\n' == lpszFormat[0])
			{
				buffer[0] = '\n';
				i = 1;
			}
			nLen = sprintf(&buffer[i], "[%08x]:", GET_TICKS()) + i;
		}
		
		if(strlen(lpszFormat) >= 1)
		{
			isNewLine = (lpszFormat[strlen(lpszFormat) - 1] == '\n');
		}
		
		nLen = vsnprintf(&buffer[nLen], bufLen - nLen, &lpszFormat[i], ptr) + nLen;

		return nLen;
	}
	else
	{
		return vsnprintf(buffer, bufLen, lpszFormat, ptr);
	}
}

int Printf_None(const char* lpszFormat, ...)
{
	return 0;
}

void _PrintfLevel(uint32 level, const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	char g_Pfbuffer[PRINTF_BUF_SIZE];
	
//	if( 0 == (level & g_dwDebugLevel)) return;
	
	va_start(ptr, lpszFormat);
	nLen = vsnprintfEx(g_Pfbuffer, sizeof(g_Pfbuffer)-1, lpszFormat, ptr);
	g_Pfbuffer[nLen] = 0;
	va_end(ptr);
	
	_puts(g_Pfbuffer);
	
}

int _Printf(const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	char g_Pfbuffer[PRINTF_BUF_SIZE];

//	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	va_start(ptr, lpszFormat);
//	nLen = vsnprintfEx(g_Pfbuffer, sizeof(g_Pfbuffer)-1, lpszFormat, ptr);
	nLen = vsnprintf(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	
	va_end(ptr);
	
	_puts(g_Pfbuffer);

	return nLen;
}


/*---------------------------------------------------*/

