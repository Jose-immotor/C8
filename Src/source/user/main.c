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
#include "Gyro.h"

int main(void)
{
	Printf("\n\nPower up.\n");	
	//MCUӲ����ʼ��
	//Todo...

	//���ж����ʼ��
	cm_backtrace_init("C7Pms", "1.0", "1.0");
	Shell_init();
	IO_Init();
	LocalTimeInit();
	Debug_Init();
	NvdsUser_Init();
	LogUser_init();
	Led_init();
	can0_init();
	JT808_init();
	Pms_init();
	Gyro_Init();
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, 1);

	//��������
	ObjList_start();
    while(1)
	{
		//��������
		ObjList_run();
		rt_thread_mdelay(1);
    }
}

