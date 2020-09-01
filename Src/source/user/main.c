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

#include "Common.h"
#include "led.h"
#include "JT808.h"
#include "Pms.h"

int main(void)
{
	//MCU硬件初始化
	//Todo...

	//所有对象初始化
	Led_init();
	JT808_init();
	Pms_init();
	
	//对象启动
	ObjList_start();
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, 1);
    while(1)
	{
		//对象运行
		ObjList_run();
    }
}

