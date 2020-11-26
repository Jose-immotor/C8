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
#include "drv_adc.h"
#include "AdcUser.h"
#include "nvc.h"
#include "cabin.h"
#include "rs485_protocol.h"
#include "drv_can.h"
#include "workmode.h"
#include "CmdLineExport.h"
#include "smart_shell.h"
#include "ButtonBoard.h"

const HwFwVer AppInfo={
FW_VER_MAIN,
FW_VER_S1,
FW_VER_S2,
FW_VER_BUILD,
DES_HW_VER_MAIN,
DES_HW_VER_SUB};


//保存WWDG计数器的设置值,默认为最大. 
//让窗口数值和计数值一样,这样就不用考虑喂狗时间在窗口值~0x3F之间了
#define WWDG_CNT 0x7f//tr   :T[6:0],计数器值 
#define WWDG_WR 0x7f//wr   :W[6:0],窗口值
//Fwwdg=PCLK1/(4096*2^fprer). 看门狗超时时间=1/720000000*8*0x40(s)=29ms
//初始化窗口看门狗
//系统时钟72MHz，
void Mcu_DgInit()
{
	rcu_periph_clock_enable(RCU_WWDGT);//   WWDG时钟使能

	wwdgt_deinit();
	wwdgt_config(WWDG_CNT, WWDG_WR, WWDGT_CFG_PSC_DIV8);               

	wwdgt_flag_clear();//清除提前唤醒中断标志位
}

void Mcu_DgStart()
{
	wwdgt_enable();
	wwdgt_flag_clear();
}

//喂窗口看门狗
void WDOG_Feed(void)
{   
	wwdgt_counter_update(WWDG_CNT);	
}

void endless_loop_for_wdTest()
{
	Printf("Wdt test.\n");

	while(1);
}

int main(void)
{
	Printf("\n\nPower up.\n");
	//MCU硬件初始化
	//Todo...
	HwFwVer_Dump(Null,&AppInfo,Null);
	//所有对象初始化
	cm_backtrace_init("C7Pms", "1.0", "1.0");
	Debug_Init();	
	LocalTimeInit();	
	NvdsUser_Init();
	LogUser_init();	
	Smart_shell_init();
	IO_Init();
	AdcUser_Init();
	Adc_init();
	Led_init();
	hw_can_init(CAN1);
	JT808_init();
	Button_init();
	Pms_init();
	Gyro_Init();
	Nvc_Init();
	Cabin_Init();
	RS485_Init();
	WorkMode_init();
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, 1);

	//对象启动
	ObjList_start();

	#ifdef DGT_CONFIG
	Mcu_DgInit();
	Mcu_DgStart();
	#endif
    while(1)
	{
		#ifdef DGT_CONFIG	
		WDOG_Feed();
		#endif
		//对象运行
		ObjList_run();
		rt_thread_mdelay(1);
    }
}

