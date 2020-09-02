/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */
 
 /*********************************************************************
ģ�鹦�ܣ�
	1��֧�ֶ������32�����Կ���λ�������п��Դ򿪻��߹ر�ָ���ĵ��Կ���λ��
		����򿪣����Կ�����Ӧ�Ŀ���λ���Ƶ���Ϣ�����
		����رգ����ܿ�����Ӧ�Ŀ���λ���Ƶ���Ϣ�����
	2��֧�ִ洢����λ������Flash�У����ô洢��
***********************************************************************/

#ifndef  _DBG_H_
#define  _DBG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#ifdef XDEBUG

#include "typedef.h"
#include "Bit.h"

extern uint32_t g_dwDebugLevel;

//���Կ���λ�ṹ���壬���ڴ�ӡ�������λ������
typedef struct _DbgSwitch
{
	uint32_t value;		//����λ��
	const char* name;	//����λ����
}DbgSwitch;

//����λ�����
#define DBG_LEV_DEF(_x) {_x, #_x}

//дFlash����ָ�롣
typedef void (*DebugWriteFn)();

	/*********************************************************************
	�������ܣ�Debugģ���ʼ������ʼ���ɹ�֮��Debug�߱����濪��λ��Flash�ʹ�ӡ����λ���ܣ�
	����������
		writeFn��д���Կ���λ��Flash�ĺ���ָ�롣
		debSwhArray�����Կ���λ���顣
		count�����Կ���λ����Ԫ������
	����ֵ����
	***********************************************************************/
	void Dbg_Init(DebugWriteFn writeFn, const DbgSwitch* debLevArray, int len);

	/*********************************************************************
	�������ܣ��򿪻��߹ر�ָ����һ�����Կ���λ��
	����������
		bitInd������λ��ţ�ȡֵ��Χ��0-31��
		isEnable������λʹ�ܡ�
	����ֵ����
	***********************************************************************/
	void Dbg_SetBit(uint32_t bitInd, Bool isEnable);

	/*********************************************************************
	�������ܣ����ö�����Կ���λ�ã�
	����������
		value������λֵ��0-0xFFFFFFFF��
	����ֵ����
	***********************************************************************/
	void Dbg_SetLevel(uint32_t value);

	//Ĭ�ϵĵ������λ����
	#define DL_MAIN 		BIT_0
	#define DL_ERROR		BIT_1
	#define DL_WARNING		BIT_2
	//�Զ���ĵ������λ���ر����BIT_5��ʼ

	#define DL_DEBUG		0xFFFFFFFF
	
	#define Assert(parenExpr) if(!(parenExpr))	\
			{                                   \
				Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #parenExpr,__FILE__,__FUNCTION__,_LINE_);	\
				while(1){;}	\
			}
			
	#define Printf(...) {printf(__VA_ARGS__); /*while(RESET == usart_flag_get(CCU_DEBUG_COM, USART_FLAG_TC));*/}
	
	#define PFL(level, ...) {if (g_dwDebugLevel & level) {Printf(__VA_ARGS__);}}
	
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
	#define Dbg_Init(...)
	#define Dbg_SetBit(...)
	#define Dbg_SetLevel(...)

	#define Printf(...)
	#define Assert(...)
	#define PFL_WARNING(...)
	#define PFL_ERROR(...)
	
	#define PFL(...)
	#define PFL_FUN_LINE(...)
	#define PFL_FILE_FUN_LINE(...)
	#define PFL_FAILED() (void)0
	#define PFL_FAILED_EXPR(...)

	#define PFL_VAR(...)
	#define WAIT(...)
#endif	//#ifdef XDEBUG

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


