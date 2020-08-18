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
//#include "Datarom.h"
#include "Smart_system.h"
#include "fsm.h"
//#include "queue.h"
#include "printf.h"
#include "drv_hwtimer.h"

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

////控制9级音量，0不响，1音量最小，8音量最大
//void Nvc_SetVol(uint8 vol)
//{
//	g_Settings.vol = vol;
//	if(g_Settings.vol > 8) g_Settings.vol = 8;
//}

void Nvc_Add(uint8 audioInd, uint8 maxRepeat)
{
	if(g_Settings.vol == 0) return;	//不响
	
	NvcItem nvcItem = {0};
	nvcItem.cmd = audioInd;
	nvcItem.maxRepeat = maxRepeat;

	//if(g_pDataRom->m_isSlience) return;

	//如果当前正在播放循环语音，则终止并且切换到下一个
	if(g_pNvcItem && NVC_LOOP == g_pNvcItem->maxRepeat)
	{
		Nvc_RemovedItem();
	}
	
	Queue_write(&g_NvcQueue, &nvcItem, sizeof(nvcItem));

//	Fsm_SetActiveFlag(AF_NVC, True);
}

#define CONVERT_TO_VOL(x) (x-1 + 0xE0)
//void Nvc_PlayEx(uint8 audioInd, uint8 maxRepeat, uint8 vol)
//{
//	Nvc_SetVol(vol);
//	
//	QUEUE_removeAll(&g_NvcQueue);
//	Nvc_Add(CONVERT_TO_VOL(g_Settings.vol), 1);
//	Nvc_Add(audioInd, maxRepeat);
//}

void Nvc_Play(uint8 audioInd, uint8 maxRepeat)
{
	if(Queue_isEmpty(&g_NvcQueue) && !g_IsSendVol)
	{
		Queue_reset(&g_NvcQueue);
		
		//控制8级音量，E0音量最小，E7音量最大，默认最大
		Nvc_Add(CONVERT_TO_VOL(g_Settings.vol), 1);
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
			//发送指令失败，在Nvc_Run中重发该命令
			Nvc_RemovedItem();
		}
	}
}

void Nvc_Reset()
{
	Queue_reset(&g_NvcQueue);
	Nvc_SetPower(False);
	Fsm_SetActiveFlag(AF_NVC, False);
}

//void Nvc_Start()
//{
////	TIMER_Start(TIMER3);
//}


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

	//发送音量
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
			//正在播放语音
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
	Queue_init(&g_NvcQueue,g_NvcItem, sizeof(NvcItem), GET_ELEMENT_COUNT(g_NvcItem));

	gpio_init(NVC_SIF_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, NVC_SIF_PIN);
	gpio_init(NVC_BSY_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, NVC_BSY_PIN);
	gpio_init(NVC_PWR_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NVC_PWR_PIN);
	
	NVC_PWR_ON();

	Sif_Init(&g_Sif, Nvc_Done);

	SwTimer_Init(&g_NvcTimer, 0, 0);
	g_pNvcItem = Null;
	g_failedCounter = 0;	
}

static void thread_nvc_entry(void* parameter)
{
	
	rt_hw_hwtimer_init();
	Nvc_Init();
	while (1)
    {
		Nvc_Run();
		rt_thread_mdelay(100);
	}

}

struct rt_thread thread_nvc;
unsigned char thread_nvc_stack[1024];
static int app_nvc_init(void)
{
	rt_err_t res;
	res=rt_thread_init(&thread_nvc,"nvc", thread_nvc_entry, RT_NULL,&thread_nvc_stack[0],
    sizeof(thread_nvc_stack), 5, 10);
    if (res == RT_EOK) /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(&thread_nvc);
	else
		rt_kprintf("\n!!create thread ble failed!\n");
    return 0;
}
INIT_APP_EXPORT(app_nvc_init);
