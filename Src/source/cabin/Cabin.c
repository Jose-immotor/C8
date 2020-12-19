
/*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : Cabin.c
 * 
 *�������12V_LOCK_ON
 *����������600ms��Ȼ�����͡�
 *��״̬��0-���������ϣ�1-������
 *
 *�������AT8837IN1��AT8847IN2��nSLEEP
 *������nSLEEP=1&IN1=1&IN2=0��ʱ1s;
 *nSLEEP=1&IN1=1&IN2=1��ʱ2s��nSLEEP=1&IN1=0&IN2=1��ʱ1s��
 *nSLEEP=1&IN1=1&IN2=1��ʱ2s��nSLEEP=0&IN1=0&IN2=0��
 *��״̬��0-���������ϣ�1-������
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
Bool g_IsCabinLock = False;				//��������ȷ��ֵ
static Bool g_IsCabinLockDetected = False;	//������ʵ�ʼ��ֵ
static Bool g_IsForceCabinLock = False;	//�û�������������״ֵ̬

Bool g_IsCabinLockFault = False;	//���������ϱ�־�������Ϳ��������û�м�⵽����״̬ʱ����Ϊ������
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
	//������ʱ������
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
		CABIN_12V_ON();//�����
		CABIN_nSLEEP_H();//�����
		CABIN_IN1_H();
		CABIN_IN2_L();
		SwTimer_Start(&g_CabinTimer, 1000, CABIN_TIME_ID1);
		Fsm_SetActiveFlag(AF_CABIN, True);
	}
}

void Cabin_Run()
{
	// ����
	if((g_pdoInfo.isCanbinLock == 1)&&(g_IsForceCabinLock))
	{
		Cabin_UnLock();
	}
	
	if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID1))
	{
		CABIN_12V_OFF();//�����
		CABIN_nSLEEP_H();//�����
		CABIN_IN1_H();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 2000, CABIN_TIME_ID2);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID2))
	{
		CABIN_nSLEEP_H();//�����
		CABIN_IN1_L();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 1000, CABIN_TIME_ID3);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID3))
	{
		CABIN_nSLEEP_H();//�����
		CABIN_IN1_H();
		CABIN_IN2_H();
		SwTimer_Start(&g_CabinTimer, 2000, CABIN_TIME_ID4);
	}
	else if(SwTimer_isTimerOut_onId(&g_CabinTimer,CABIN_TIME_ID4))
	{
		CABIN_nSLEEP_L();//�����
		CABIN_IN1_L();
		CABIN_IN2_L();
		Fsm_SetActiveFlag(AF_CABIN, False);
		g_IsCabinLockFault = g_IsCabinLock;
	}

	// ������
	if(g_IsCabinLockFault)
	{
		static uint32 tick = 0;
		if(SwTimer_isTimerOutEx(tick, 2000))
		{
			tick = GET_TICKS();
			//Nvc_Play(NVC_WARNING, 1);	//�澯��ʾ
		}
	}

	// 
	if(SwTimer_isTimerOut(&g_CabinLockStateTimer))
	{
		PFL(DL_CABIN,"canbin lock=%d,%d\n", g_IsCabinLockDetected, g_IsCabinLock);
		if(g_IsCabinLockDetected == CABIN_IS_LOCK && g_IsCabinLock != g_IsCabinLockDetected)
		{
			Nvc_Play(NVC_INFO, 1);	//��ʾ��
			g_IsCabinLock = g_IsCabinLockDetected;
			g_IsCabinLockFault = False;	//�������״̬��
//			PostMsg(MSG_CABIN_LOCK_CHANGED);
			//�ж��Ƿ�Ƿ�������û�нӵ���������ȴ��⵽����
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
	g_pCabin12VOnIO = IO_Get(IO_CABIN_12V_ON);//�����
	g_pCabinIN1IO = IO_Get(IO_AT8837_IN1);//�����
	g_pCabinIN2IO = IO_Get(IO_AT8837_IN2);
	g_pCabinnSLEEPIO = IO_Get(IO_AT8837_nSLEEP);
		
	g_IsCabinLock = CABIN_IS_LOCK;
	g_IsCabinLockDetected = g_IsCabinLock;
	g_IsForceCabinLock = g_IsCabinLock;
	PFL(DL_CABIN,"CabinLock=%s\n", g_IsCabinLock ? "Lock" : "UnLock");
}
#endif
