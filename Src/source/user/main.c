/*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : main.c
 * 
 * Change Logs:
 * Date		      Author		Notes
 * 2020-07-28	  lane 	 	    first implementation
 * 2020-08-27	  Allen	 	    modified
*******************************************************************************/

#include "pms.h"
#include "Common.h"
#include "led.h"
#include "JT808.h"
#include <cm_backtrace.h>
#include "Debug.h"
#include "drv_rtc.h"
#include "drv_gpio.h"

int main(void)
{
	Printf("\n\nPower up.\n");	
	//MCU硬件初始化
	//Todo...

	//所有对象初始化
	cm_backtrace_init("C7Pms", "1.0", "1.0");
	IO_Init();
	LocalTimeInit();
	Debug_Init();
	NvdsUser_Init();
	LogUser_init();
	Led_init();
//	JT808_init();
	Pms_init();
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, 1);

	//对象启动
	ObjList_start();
    while(1)
	{
		//对象运行
		ObjList_run();
		rt_thread_mdelay(100);
    }
}

