
#ifndef  _XDEBUG_H_
#define  _XDEBUG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#include "typedef.h"
#include "Bit.h"

extern uint32_t g_dwDebugLevel;
	
#ifdef XDEBUG
	void Debug_Init(void);
	void ShowWait(int printDotPerCount, int dotCountPerRow);
	void Debug_SetBit(uint32_t bitInd, Bool isEnable);
	void Debug_SetLevel(uint8_t level);

	#define DL_MAIN 		BIT_0
	#define DL_ERROR		BIT_1
	#define DL_WARNING		BIT_2

	#define DL_IO			BIT_4
	#define DL_ADC			BIT_5

	#define DL_DEBUG		0xFFFFFFFF
	
	#define Assert(parenExpr) if(!(parenExpr))	\
			{                                   \
				Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #parenExpr,__FILE__,__FUNCTION__,_LINE_);	\
				while(1){;}	\
			}
			
	#define Printf(...) {printf(__VA_ARGS__); /*while(RESET == usart_flag_get(CCU_DEBUG_COM, USART_FLAG_TC));*/}
	
	//#define Trace Printf
	#define PFL(level, ...) {if (g_dwDebugLevel & level) {Printf(__VA_ARGS__);}}

	#define SHELL(parenExpr) Printf parenExpr
	
	#define PFL_WARNING(...) 	PFL(DL_WARNING, "WARNING: %s(),line=%d: ",_FUNC_, _LINE_); PFL(DL_WARNING, __VA_ARGS__);
	#define PFL_ERROR(...) 	PFL(DL_ERROR, "ERROR: %s(),line=%d:",_FUNC_, _LINE_); PFL(DL_ERROR, __VA_ARGS__);
	//#define PFL_ERROR(parenExpr) 	PFL(DL_ERROR, "ERROR: line=%d:",_LINE_); PFL(DL_ERROR, parenExpr);

	#define PFL_FUN_LINE(level) PFL(level, "%s(),line=%d\n",_FUNC_, _LINE_);
	#define PFL_FILE_FUN_LINE(level) PFL(level, ("PFL Line. %s,%s(),line=%d\n", __FILE__,__FUNCTION__,_LINE_))
	#define PFL_VAR(V1) 		Printf("%s(),line=%d,%s=[0x%x](%d)\n",_FUNC_, _LINE_, #V1, V1, V1)
	#define PFL_FAILED() 		PFL(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d.\n",_FUNC_, _LINE_))
	#define PFL_FAILED_EXPR(parenExpr) 		PFL(DL_WARNING, ("%s() FAILED,line=%d:",_FUNC_, _LINE_)); PFL(DL_WARNING, parenExpr);
	
	#define WAIT(maxMS, parenExpr) {int ms = 0; while(!(parenExpr) && ms++ < (maxMS)) {SLEEP(1);}}

#else	//#ifdef XDEBUG
	#define Printf(...)
	#define DUG_BIT_0(...)
	#define DUG_BIT_1(...)			
	#define Debug_SetBit(...)
	#define Debug_SetLevel(...)
	#define Debug_Init(...)
	#define ShowWait(...)
	#define Debug_EnbaleLevel(...)
	#define Printf(...)
	#define ASRT(...)
	#define ASSERT_FALSE(...)
	#define ASSERT_EX(...)
	#define Trace(...)
	#define PFL_WARNING(...)
	#define PFL_ERROR(...)
	
	#define PFL(...)
	#define PFL_FUN_LINE(...)
	#define PFL_FILE_FUN_LINE(...)
	#define PFL_FAILED() (void)0
	#define PFL_FAILED_EXPR(...)

	#define Debug_DumpByteEx(...)
	#define Debug_DumpByte(...)
	#define Debug_DumpDword(...)

	#define DUMP_BYTE(...)
	#define DUMP_DWORD(...)

	#define DUMP_BYTE_EX(...)
	#define DUMP_BYTE_LEVEL(...)
	#define DUMP_DWORD_LEVEL(...)
	#define DUMP_BYTE_LEVEL_EX(...)
	#define Debug_Init(...)

	#define SHELL(...)
	#define PFL_VAR(...)
	#define WAIT(...)
	#define Debug_Set(...)
#endif	//#ifdef XDEBUG

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


