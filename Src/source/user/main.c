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
_GENERAL_CENTRAL_CTL		// ��ͨ�п�,�ӵ�ؾͷŵ�



*/

//#if ( VECT_TAB_OFFSET > 0 )


#define     HW_VER_ADDR         ( 0x8004000 - 128 + 8 ) 


const HwFwVer AppInfo={
	FW_VER_MAIN,
	FW_VER_S1,
	FW_VER_S2,
	FW_VER_BUILD,
	DES_HW_VER_MAIN,
	DES_HW_VER_SUB
};


/*
	CANBUS_MODE_JT808_ENABLE : ֧������ģ��
	NON_SHARE_ELECTRIC_VEHICLES : ����Ӫ���� 
	Special Edition
	SPECIAL_EDITION_AUTOACCON	: ����汾,������ֱ�ӷŵ�
*/
//
//
#if 0
//����WWDG������������ֵ,Ĭ��Ϊ���? 
//�ô�����ֵ�ͼ���ֵһ��,�����Ͳ��ÿ���ι��ʱ���ڴ���ֵ~0x3F֮����
#define WWDG_CNT 0x7f//tr   :T[6:0],������ֵ 
#define WWDG_WR 0x7f//wr   :W[6:0],����ֵ
//Fwwdg=PCLK1/(4096*2^fprer). ���Ź���ʱʱ��=1/720000000*8*0x40(s)=29ms
//��ʼ�����ڿ��Ź�
//ϵͳʱ��72MHz��

// ʹ�ö������Ź�

void Mcu_DgInit()
{
	rcu_periph_clock_enable(RCU_WWDGT);//   WWDGʱ��ʹ��

	wwdgt_deinit();
	wwdgt_config(WWDG_CNT, WWDG_WR, WWDGT_CFG_PSC_DIV8);               

	wwdgt_flag_clear();//�����ǰ�����жϱ�־λ
}

void Mcu_DgStart()
{
	wwdgt_enable();
	wwdgt_flag_clear();
}

//ι���ڿ��Ź�
void WDOG_Feed(void)
{   
	wwdgt_counter_update(WWDG_CNT);	
}
#else
static void Mcu_DgInit(void)
{
	if(RESET != rcu_flag_get(RCU_FLAG_FWDGTRST))
	{
		/* clear the FWDGT reset flag */
	    rcu_all_reset_flag_clear();
	}
	/* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    
    /* wait till IRC40K is ready */
    while(1 != rcu_osci_stab_wait(RCU_IRC40K)){
    }
	//fwdgt_write_enable();
	/* confiure FWDGT counter clock: 40KHz(IRC40K) / 8 = 0.625 KHz */
    fwdgt_config(4000,FWDGT_PSC_DIV32);
}

static void Mcu_DgStart(void)
{
    /* after 2.0 seconds to generate a reset */
    fwdgt_enable();
}

void WDOG_Feed(void)
{
	fwdgt_counter_reload();
}



#endif 

void endless_loop_for_wdTest()
{
	Printf("Wdt test.\n");

	while(1);
}

int main(void)
{
	__enable_irq();
	Printf("\n\nPower up\n");
	//MCUӲ����ʼ��
	//Todo...
	HwFwVer_Dump("C8",&AppInfo,Null);
	cm_backtrace_init("C8Pms", "1.0", "1.0");
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

	//��������
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
		//��������
		ObjList_run();
		rt_thread_mdelay(1);
    }
}

