/*
 * File      : debug.h
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-08     lane      first implementation
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C"{
#endif
#include "common.h"
#include<stdint.h>
#include<stdbool.h>

extern uint32_t g_dwDebugLevel;
#ifdef XDEBUG
	#include "typedef.h"
	#include "printf.h"

	void Debug_Init(void);
	void ShowWait(int printDotPerCount, int dotCountPerRow);
//	void Debug_SetBit(uint32_t bitInd, bool isEnable);
//	void Debug_SetLevel(uint8_t level);

	#define DL_MAIN 		BIT_0
	#define DL_ERROR		BIT_1
	#define DL_WARNING		BIT_2
	#define DL_ASSERT		BIT_3
						
	#define DL_BLE			BIT_5
	#define DL_GPS			BIT_6
	#define DL_GPRS			BIT_7
	#define DL_SIM			BIT_8
	#define DL_ATCCMD		BIT_9
	#define DL_GYRO			BIT_10
	#define DL_PMS			BIT_11
	#define DL_FSM			BIT_12
	#define DL_ADC			BIT_13
	#define DL_LOG			BIT_14
	#define DL_BEACON		BIT_15
	#define DL_SY6990		BIT_16
	
	#define DL_SHOW_CALLED	BIT_30
	#define DL_SHOW_TICKS	BIT_31
	#define DL_DEBUG		0xFFFFFFFF
	#define DUG_BIT_0(dl)	(g_dwDebugLevel &= ~dl)
	#define DUG_BIT_1(dl)	(g_dwDebugLevel |= dl)			
	
	#define ASRT(parenExpr) if((g_dwDebugLevel & DL_ASSERT) && !(parenExpr))	\
			{                                   \
				Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #parenExpr,__FILE__,__FUNCTION__,_LINE_);	\
				while(1){;}	\
			}
	#define ASSERT_FALSE(parenExpr) {	\
				Printf parenExpr;	\
				ASRT(FALSE);\
			}
	
	//#define Trace Printf
	#define PFL _PrintfLevel

	void Debug_DumpByteEx(uint32_t level, const uint8_t* pData, uint16_t len, uint8_t cols, const char* str, int line) ;
	void Debug_DumpByte(const uint8_t* pData, uint16_t len, uint8_t cols);
	void Debug_DumpDword(const uint32_t* pData, uint16_t len, uint8_t cols);

	#define DUMP_BYTE(address,len)  Debug_DumpByte((uint8_t*)address, len, 32)
	#define DUMP_BYTE_EX(str,address,len)  {Printf str;Debug_DumpByte((uint8_t*)address, len, 32);}
	#define DUMP_DWORD(address,len) Debug_DumpDword(address, len, 8)

	#define DUMP_BYTE_LEVEL_EX(level, address, len, cols) {\
		if(((uint32_t)(level)) & g_dwDebugLevel) \
		{	\
    		Debug_DumpByte(address, len, cols);	\
		}	\
	}

	#define DUMP_BYTE_LEVEL(level, address, len){\
		if(((uint32_t)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_BYTE(address,len);	\
		}	\
	}
	
	#define DUMP_BYTE_EX_LEVEL(level, str, address, len){\
		if(((uint32_t)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_BYTE_EX(str, address,len);	\
		}	\
	}
	
	#define DUMP_DWORD_LEVEL(level, address,len){\
		if(((uint32_t)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_DWORD(address,len);	\
		}	\
	}

	#define SHELL(parenExpr) Printf parenExpr
	
	#define PFL_WARNING(parenExpr) 	PFL(DL_WARNING, ("WARNING: %s(),line=%d: ",_FUNC_, _LINE_)); PFL(DL_WARNING, parenExpr);
	#define PFL_ERROR(parenExpr) 	PFL(DL_ERROR, ("ERROR: %s(),line=%d:",_FUNC_, _LINE_)); PFL(DL_ERROR, parenExpr);

	#define PFL_FUN_LINE(level) PFL(level, "%s(),line=%d\n",_FUNC_, _LINE_);
	#define PFL_FILE_FUN_LINE(level) PFL(level, ("PFL Line. %s,%s(),line=%d\n", __FILE__,__FUNCTION__,_LINE_))
	#define PFL_VAR(V1) 		Printf("%s(),line=%d,%s=[0x%x](%d)\n",_FUNC_, _LINE_, #V1, V1, V1)
	#define PFL_FAILED() 		PFL(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d.\n",_FUNC_, _LINE_))
	#define PFL_FAILED_EXPR(parenExpr) 		PFL(DL_WARNING, ("%s() FAILED,line=%d:",_FUNC_, _LINE_)); PFL(DL_WARNING, parenExpr);
	
	#define WAIT(maxMS, parenExpr) {int ms = 0; while(!(parenExpr) && ms++ < (maxMS)) {SLEEP(1);}}

#else	//#ifdef XDEBUG
	#define DUG_BIT_0(...)
	#define DUG_BIT_1(...)			
	#define Debug_SetBit(...)
	#define Debug_SetLevel(...)
	#define Debug_Init(...)
	#define ShowWait(...)
	#define Debug_EnbaleLevel(...)
//	#define Printf(...)
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
#endif

#endif
