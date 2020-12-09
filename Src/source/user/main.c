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

#ifdef CANBUS_MODE_JT808_ENABLE
#include "JT808.h"
#endif //CANBUS_MODE_JT808_ENABLE


/*
General central control

// 
_GENERAL_CENTRAL_CTL		// ÆÕÍ¨ÖÐ¿Ø,½Óµç³Ø¾Í·Åµç



*/

const HwFwVer AppInfo={
FW_VER_MAIN,
FW_VER_S1,
FW_VER_S2,
FW_VER_BUILD,
DES_HW_VER_MAIN,
DES_HW_VER_SUB};

/*
	CANBUS_MODE_JT808_ENABLE : Ö§³ÖÍâÖÃÄ£¿é
	NON_SHARE_ELECTRIC_VEHICLES : ·ÇÔËÓª³µÁ¾ 
	Special Edition
	SPECIAL_EDITION_AUTOACCON	: ÌØÊâ°æ±¾,·ÅÈëµç³ØÖ±½Ó·Åµç
*/
//
//
//ï¿½ï¿½ï¿½ï¿½WWDGï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Öµ,Ä¬ï¿½ï¿½Îªï¿½ï¿½ï¿? 
//ï¿½Ã´ï¿½ï¿½ï¿½ï¿½ï¿½Öµï¿½Í¼ï¿½ï¿½ï¿½ÖµÒ»ï¿½ï¿½,ï¿½ï¿½ï¿½ï¿½ï¿½Í²ï¿½ï¿½Ã¿ï¿½ï¿½ï¿½Î¹ï¿½ï¿½Ê±ï¿½ï¿½ï¿½Ú´ï¿½ï¿½ï¿½Öµ~0x3FÖ®ï¿½ï¿½ï¿½ï¿½
#define WWDG_CNT 0x7f//tr   :T[6:0],ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Öµ 
#define WWDG_WR 0x7f//wr   :W[6:0],ï¿½ï¿½ï¿½ï¿½Öµ
//Fwwdg=PCLK1/(4096*2^fprer). ï¿½ï¿½ï¿½Å¹ï¿½ï¿½ï¿½Ê±Ê±ï¿½ï¿½=1/720000000*8*0x40(s)=29ms
//ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½Ú¿ï¿½ï¿½Å¹ï¿½
//ÏµÍ³Ê±ï¿½ï¿½72MHzï¿½ï¿½
void Mcu_DgInit()
{
	rcu_periph_clock_enable(RCU_WWDGT);//   WWDGÊ±ÖÓÊ¹ÄÜ

	wwdgt_deinit();
	wwdgt_config(WWDG_CNT, WWDG_WR, WWDGT_CFG_PSC_DIV8);               

	wwdgt_flag_clear();//Çå³ýÌáÇ°»½ÐÑÖÐ¶Ï±êÖ¾Î»
}

void Mcu_DgStart()
{
	wwdgt_enable();
	wwdgt_flag_clear();
}

//Î¹´°¿Ú¿´ÃÅ¹·
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
	__enable_irq();
	Printf("\n\nPower up\n");
	//MCUÓ²ï¿½ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½
	//Todo...
	HwFwVer_Dump(Null,&AppInfo,Null);
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
#ifdef CANBUS_MODE_JT808_ENABLE	
	hw_can_init(CAN1);
	JT808_init();
	//Jt808TaskInit();
#endif //CANBUS_MODE_JT808_ENABLE
	//Button_init();
	Pms_init();
	Gyro_Init();
	Nvc_Init();
	Cabin_Init();
	RS485_Init();
	WorkMode_init();
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_McuReset, 1);

	//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
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
		//¶ÔÏóÔËÐÐ
		ObjList_run();
		rt_thread_mdelay(1);
    }
}

