
#ifndef  _XDEBUG_H_
#define  _XDEBUG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#include "Dbg.h"

/***********************
BIT_0 - BIT_3
保留为系统使用
***********************/

#define DL_CAN 		BIT_4
#define DL_IO		BIT_5
#define DL_ADC		BIT_6
#define DL_LOG		BIT_7
#define DL_PMS		BIT_8
#define DL_MDB		BIT_9
#define DL_NFC		BIT_10
#define DL_GYRO		BIT_11
#define DL_JT808	BIT_12
#define DL_CABIN	BIT_13

void Debug_Init();

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


