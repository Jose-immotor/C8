/*
 * File      : nvc.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-12      lane      first implementation
 */
#include "Common.h"
#include "Nvc.h"
#include "Sif.h"
#include "drv_hwtimer.h"

static DrvIo* g_pPwrNvcEnIO = Null;

#define NVC_PWR_ON()	PortPin_Set(g_pPwrNvcEnIO->periph, g_pPwrNvcEnIO->pin, True)
#define NVC_PWR_OFF()	PortPin_Set(g_pPwrNvcEnIO->periph, g_pPwrNvcEnIO->pin, False)

#define NVC_IS_BSY() 	(IO_Read(IO_NVC_BUSY) == RESET)

static const uint32 CLK_US = 100;

static Sif g_Sif;

static NvcItem* g_pNvcItem = Null;
static Queue 	g_NvcQueue;
static  uint8 	g_failedCounter;
static SwTimer	g_NvcTimer;

static uint8 g_IsSendVol = False;

void TIMER3_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP)!=RESET)
	{
		Sif_Isr(&g_Sif, CLK_US);
		timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
	}
}

void Nvc_RemovedItem()
{	
	SwTimer_Stop(&g_NvcTimer);
	
	g_failedCounter = 0;
	if(g_pNvcItem == Queue_Read(&g_NvcQueue))
	{
		Queue_reset(&g_NvcQueue);
	}
	g_pNvcItem = Null;
}

void Nvc_SetPower(Bool isOn)
{
	if(isOn)
	{
		gpio_init(NVC_SIF_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NVC_SIF_PIN);
		NVC_PWR_ON();
	}
	else
	{
		g_IsSendVol = False;		
		NVC_PWR_OFF();
		gpio_init(NVC_SIF_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, NVC_SIF_PIN);
	}
	Printf("Nvc Power%s\n", isOn ? "On" : "Off");
}

Bool Nvc_IsPwrOn()
{
	return (gpio_output_bit_get(NVC_PWR_PORT,NVC_PWR_PIN) == SET);
}

//����9��������0���죬1������С��8�������
void Nvc_SetVol(uint8 vol)
{
	g_cfgInfo.vol = vol;
	if(g_cfgInfo.vol > 8) g_cfgInfo.vol = 8;
}

void Nvc_Add(uint8 audioInd, uint8 maxRepeat)
{
	if(g_cfgInfo.vol == 0) return;	//����
	
	NvcItem nvcItem = {0};
	nvcItem.cmd = audioInd;
	nvcItem.maxRepeat = maxRepeat;

	//if(g_pDataRom->m_isSlience) return;

	//�����ǰ���ڲ���ѭ������������ֹ�����л�����һ��
	if(g_pNvcItem && NVC_LOOP == g_pNvcItem->maxRepeat)
	{
		Nvc_RemovedItem();
	}
	
	Queue_write(&g_NvcQueue, &nvcItem, sizeof(nvcItem));

	Fsm_SetActiveFlag(AF_NVC, True);
}

#define CONVERT_TO_VOL(x) (x-1 + 0xE0)
void Nvc_PlayEx(uint8 audioInd, uint8 maxRepeat, uint8 vol)
{
	Nvc_SetVol(vol);
	
	Queue_reset(&g_NvcQueue);
	Nvc_Add(CONVERT_TO_VOL(g_cfgInfo.vol), 1);
	Nvc_Add(audioInd, maxRepeat);
}

void Nvc_Play(uint8 audioInd, uint8 maxRepeat)
{
	if(Queue_isEmpty(&g_NvcQueue) && !g_IsSendVol)
	{
		Queue_reset(&g_NvcQueue);
		
		//����8��������E0������С��E7�������Ĭ�����
		Nvc_Add(CONVERT_TO_VOL(g_cfgInfo.vol), 1);
	}
	Nvc_Add(audioInd, maxRepeat);
}

void Nvc_Done(Sif* pSif)
{
	//Printf("Nvc_Done[%x].\n", g_pNvcItem->cmd);
	SwTimer_Start(&g_NvcTimer, 10000, 0);
	if(NVC_IS_BSY())
	{
		g_failedCounter = 0;
		if(NVC_LOOP == g_pNvcItem->maxRepeat)
		{
			Sif_TxByte(&g_Sif, 0xF2);
		}
	}
	else if(g_pNvcItem->cmd >= 0xE0 && g_pNvcItem->cmd <= 0xE7)
	{
		g_IsSendVol = True;
		g_failedCounter = 0;
		Nvc_RemovedItem();
	}
	else 
	{
		++g_failedCounter;
		g_pNvcItem = Null;
		if(g_failedCounter >= 3)
		{
			//����ָ��ʧ�ܣ���Nvc_Run���ط�������
			Nvc_RemovedItem();
		}
	}
}

void Nvc_Reset()
{
	Queue_reset(&g_NvcQueue);
	Nvc_SetPower(False);
//	Fsm_SetActiveFlag(AF_NVC, False);
}

static void Nvc_Start(void)
{
//	timer_enable(TIMER3);

	NVC_PWR_ON();

	Sif_Init(&g_Sif, Nvc_Done);

	SwTimer_Init(&g_NvcTimer, 0, 0);
	g_pNvcItem = Null;
	g_failedCounter = 0;
	rt_hw_hwtimer_init();

}

static void Nvc_Stop(void)
{
	NVC_PWR_OFF();
    timer_deinit(TIMER3);
	rcu_periph_clock_disable(RCU_TIMER3);
}

void Nvc_SendNvcItem(NvcItem* pNvcItem)
{
	if(!Nvc_IsPwrOn())
	{
		Nvc_SetPower(True);
		rt_thread_mdelay(300);
	}
	
	Sif_TxByte(&g_Sif, g_pNvcItem->cmd);
}

void Nvc_Run()
{
	if(!Sif_isDone(&g_Sif)) return;

	//��������
	if(SwTimer_isTimerOut(&g_NvcTimer))
	{
		Nvc_RemovedItem();
		return;
	}
	if(g_pNvcItem == Null)
	{
		g_pNvcItem = Queue_Read(&g_NvcQueue);
		if(g_pNvcItem)
		{
			Nvc_SendNvcItem(g_pNvcItem);
			Queue_pop(&g_NvcQueue);
		}
		else if(Nvc_IsPwrOn())
		{
			Nvc_SetPower(False);
			Fsm_SetActiveFlag(AF_NVC, False);
		}
	}
	else
	{
		if(NVC_IS_BSY())
		{
			//���ڲ�������
		}
		else
		{
			if(++g_pNvcItem->repeatCounter >= g_pNvcItem->maxRepeat)
			{
				Nvc_RemovedItem();
			}
			else
			{
				Sif_TxByte(&g_Sif, g_pNvcItem->cmd);
			}
		}
	}
}

void Nvc_Init()
{
	static NvcItem g_NvcItem[5];
	const static Obj obj = {
	.name = "NVC",
	.Stop = Nvc_Stop,
	.Start = Nvc_Start,
	.Run = Nvc_Run,
	};

	ObjList_add(&obj);
	
	Queue_init(&g_NvcQueue,g_NvcItem, sizeof(NvcItem), GET_ELEMENT_COUNT(g_NvcItem));
	
	g_pPwrNvcEnIO = IO_Get(IO_NVC_PWR);
	
	
}
