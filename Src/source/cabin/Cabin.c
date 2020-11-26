
/*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : Cabin.c
 * 
 *电磁锁：12V_LOCK_ON
 *开锁：拉高600ms，然后拉低。
 *锁状态：0-关锁、锁上；1-开锁。
 *
 *电机锁：AT8837IN1、AT8847IN2、nSLEEP
 *开锁：nSLEEP=1&IN1=1&IN2=0延时1s;
 *nSLEEP=1&IN1=1&IN2=1延时2s。nSLEEP=1&IN1=0&IN2=1延时1s。
 *nSLEEP=1&IN1=1&IN2=1延时2s。nSLEEP=0&IN1=0&IN2=0。
 *锁状态：0-关锁、锁上；1-开锁。
 * Change Logs:
 * Date		      Author		Notes
 * 2020-09-28	  lane 	 	    first implementation
*******************************************************************************/

#include "Common.h"
#include "Cabin.h"
#include "Nvc.h"

#ifdef CFG_CABIN_LOCK

#define CABIN_TIME_ID1	0
#define CABIN_TIME_ID2	1
#define CABIN_TIME_ID3	2
#define CABIN_TIME_ID4	3
#define CABIN_TIME_ID5	4

static SwTimer g_CabinTimer;
static SwTimer g_CabinLockStateTimer;
Bool g_IsCabinLock = False;				//座舱锁的确定值
static Bool g_IsCabinLockDetected = False;	//座舱锁实际检测值
static Bool g_IsForceCabinLock = False;	//用户设置座舱锁的状态值

Bool g_IsCabinLockFault = False;	//座舱锁故障标志，当发送开锁命令而没有检测到开锁状态时，认为锁故障
static DrvIo* g_pCabin12VOnIO = Null;
#define CABIN_12V_ON()	PortPin_Set(g_pCabin12VOnIO->periph, g_pCabin12VOnIO->pin, True)
#define CABIN_12V_OFF()	PortPin_Set(g_pCabin12VOnIO->periph, g_pCabin12VOnIO->pin, False)

static DrvIo* g_pCabinIN1IO = Null;
#define CABIN_IN1_H()	PortPin_Set(g_pCabinIN1IO->periph, g_pCabinIN1IO->pin, True)
#define CABIN_IN1_L()	PortPin_Set(g_pCabinIN1IO->periph, g_pCabinIN1IO->pin, False)
static DrvIo* g_pCabinIN2IO = Null;
#define CABIN_IN2_H()	PortPin_Set(g_pCabinIN2IO->periph, g_pCabinIN2IO->pin, True)
#define CABIN_IN2_L()	PortPin_Set(g_pCabinIN2IO->periph, g_pCabinIN2IO->pin, False)
static DrvIo* g_pCabinnSLEEPIO = Null;
#define CABIN_nSLEEP_H()	PortPin_Set(g_pCabinnSLEEPIO->periph, g_pCabinnSLEEPIO->pin, True)
#define CABIN_nSLEEP_L()	PortPin_Set(g_pCabinnSLEEPIO->periph, g_pCabinnSLEEPIO->pin, False)

void Cabin_Dump(void)
{
	Printf("Cabin info:\n");
	Printf("\tg_IsCabinLock=%d\n", g_IsCabinLock);
	Printf("\tg_IsCabinLockDetected=%d\n", g_IsCabinLockDetected);
	Printf("\tg_IsForceCabinLock=%d\n", g_IsForceCabinLock);

}

void Mcu_CabinLockIsr()
{
	rt_interrupt_enter();
	g_IsCabinLockDetected = CABIN_IS_LOCK;
	//启动定时器防抖
	SwTimer_Start(&g_CabinLockStateTimer, 500, 0);
	if(g_isPowerDown)	
	{
		SetWakeUpType(WAKEUP_CABIN_LOCK);
	}
	rt_interrupt_leave();
}

void Cabin_UnLock()
{
	g_IsForceCabinLock = False;
	if(!g_CabinTimer.m_isStart)
	{
		CABIN_12V_ON();//电磁锁
		CABIN_nSLEEP_H();//电机锁
		CABIN_IN1_H();
		CABIN_IN2_L();
		SwTimer_Start(&g_CabinTimer, 1000, CABIN_TIME_ID1);
		Fsm_SetActiveFlag(AF_CABIN, True);
	}
}

void Cabin_Run()
{
	if((g_pdoInfo.isCanbinLock == 1)&&(g_IsForceCabinLock))
	{
		Cabin_UnLock();
	}
	
	if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID1))
	{
		CABIN_12V_OFF();//电磁锁
		CABIN_nSLEEP_H();//电机锁
		CABIN_IN1_H();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 2000, CABIN_TIME_ID2);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID2))
	{
		CABIN_nSLEEP_H();//电机锁
		CABIN_IN1_L();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 1000, CABIN_TIME_ID3);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID3))
	{
		CABIN_nSLEEP_H();//电机锁
		CABIN_IN1_H();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 2000, CABIN_TIME_ID4);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID4))
	{
		CABIN_nSLEEP_L();//电机锁
		CABIN_IN1_L();
		CABIN_IN2_L();
		Fsm_SetActiveFlag(AF_CABIN, False);
		g_IsCabinLockFault = g_IsCabinLock;
	}
	
	if(g_IsCabinLockFault)
	{
		static uint32 tick = 0;
		if(SwTimer_isTimerOutEx(tick, 2000))
		{
			tick = GET_TICKS();
			Nvc_Play(NVC_WARNING, 1);	//告警提示
		}
	}
	
	if(SwTimer_isTimerOut(&g_CabinLockStateTimer))
	{
		PFL(DL_CABIN,"canbin lock=%d,%d\n", g_IsCabinLockDetected, g_IsCabinLock);
		if(g_IsCabinLockDetected == CABIN_IS_LOCK && g_IsCabinLock != g_IsCabinLockDetected)
		{
			Nvc_Play(NVC_INFO, 1);	//提示音
			g_IsCabinLock = g_IsCabinLockDetected;
			g_IsCabinLockFault = False;	//清除故障状态。
//			PostMsg(MSG_CABIN_LOCK_CHANGED);
			//判断是否非法开锁，没有接到开锁命令却检测到开锁
			if(!g_IsCabinLock && g_IsForceCabinLock != g_IsCabinLock)
			{
				Nvc_Play(NVC_BAT_UNSAFE, 1);
			}
			g_IsForceCabinLock = g_IsCabinLock;
			PFL(DL_CABIN,"CabinLock=%s\n", g_IsCabinLock ? "Lock" : "UnLock");
		}
	}
}

void Cabin_Init()
{
	const static Obj obj = {
	.name = "Cabin",
	.Run = Cabin_Run,
	};

	ObjList_add(&obj);
	g_pCabin12VOnIO = IO_Get(IO_CABIN_12V_ON);//电磁锁
	g_pCabinIN1IO = IO_Get(IO_AT8837_IN1);//电机锁
	g_pCabinIN2IO = IO_Get(IO_AT8837_IN2);
	g_pCabinnSLEEPIO = IO_Get(IO_AT8837_nSLEEP);
		
	g_IsCabinLock = CABIN_IS_LOCK;
	g_IsCabinLockDetected = g_IsCabinLock;
	g_IsForceCabinLock = g_IsCabinLock;
	PFL(DL_CABIN,"CabinLock=%s\n", g_IsCabinLock ? "Lock" : "UnLock");
}
#endif
