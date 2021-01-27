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

#define			_PALY_TIMEOUT_ID		1
#define			_BUSY_TIMEOUT_ID		2

static uint8 g_IsSendVol = False;

void TIMER3_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP)!=RESET)
	{
		timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
		Sif_Isr(&g_Sif, CLK_US);
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
void Nvc_RemovedItem_2()
{	
	SwTimer_Stop(&g_NvcTimer);
	
	g_failedCounter = 0;
	//if(g_pNvcItem == Queue_Read(&g_NvcQueue))
	//{
	//	Queue_reset(&g_NvcQueue);
	//}
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

//控制9级音量，0不响，1音量最小，8音量最大
void Nvc_SetVol(uint8 vol)
{
	g_cfgInfo.vol = vol;
	if(g_cfgInfo.vol > 8) g_cfgInfo.vol = 8;
}

void Nvc_Add(uint8 audioInd, uint8 maxRepeat)
{
	if(g_cfgInfo.vol == 0) return;	//不响
	
	NvcItem nvcItem = {0};
	nvcItem.cmd = audioInd;
	nvcItem.maxRepeat = maxRepeat;

	//if(g_pDataRom->m_isSlience) return;

	//如果当前正在播放循环语音，则终止并且切换到下一个
	if(g_pNvcItem && NVC_LOOP == g_pNvcItem->maxRepeat)
	{
		Nvc_RemovedItem();
	}
	
	if( Queue_write(&g_NvcQueue, &nvcItem, sizeof(nvcItem)) )
	{
		Fsm_SetActiveFlag(AF_NVC, True);

		//Printf("Nvc Add:%d[%d-%d]\n",audioInd,g_NvcQueue.readInd,g_NvcQueue.writeInd );
	}
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
		
		//控制8级音量，E0音量最小，E7音量最大，默认最大
		Nvc_Add(CONVERT_TO_VOL(g_cfgInfo.vol), 1);
	}
	Nvc_Add(audioInd, maxRepeat);
}

void Nvc_Done(Sif* pSif)
{
	//Printf("Nvc_Done[%x].\n", g_pNvcItem->cmd);
	//SwTimer_Start(&g_NvcTimer, 10000, 0);
	SwTimer_Start(&g_NvcTimer, 500, _BUSY_TIMEOUT_ID );	// 等待2s后再开始检测BUSY
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
		//g_pNvcItem = Null;
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
//	Fsm_SetActiveFlag(AF_NVC, False);
}

static void Nvc_Start(void)
{
//	timer_enable(TIMER3);

	//NVC_PWR_ON();

	Sif_Init(&g_Sif, Nvc_Done);

	SwTimer_Init(&g_NvcTimer, 0, 0);
	g_pNvcItem = Null;
	g_failedCounter = 0;
	rt_hw_hwtimer_init();

}

static void Nvc_Stop(void)
{
	SwTimer_Stop(&g_NvcTimer);
	NVC_PWR_OFF();
    timer_deinit(TIMER3);
	rcu_periph_clock_disable(RCU_TIMER3);
	Fsm_SetActiveFlag(AF_NVC, False);
}

void Nvc_SendNvcItem(NvcItem* pNvcItem)
{
	if(!Nvc_IsPwrOn())
	{
		Nvc_SetPower(True);
		rt_thread_mdelay(300);
	}
	
	Sif_TxByte(&g_Sif, g_pNvcItem->cmd);
	SwTimer_Start(&g_NvcTimer, 10*1000, _PALY_TIMEOUT_ID );
}

void Nvc_Run()
{
	if(!Sif_isDone(&g_Sif)) return;

	//发送音量
	//if(SwTimer_isTimerOut(&g_NvcTimer))
	if( SwTimer_isTimerOut_onId(&g_NvcTimer , _PALY_TIMEOUT_ID ) )		// 整个超时
	{
		Nvc_RemovedItem();
		//Printf("NVC Timeout\n");
		return;
	}

	if( SwTimer_IsStart_onId( &g_NvcTimer , _BUSY_TIMEOUT_ID ) )
	{
		if( SwTimer_isTimerOut_onId(&g_NvcTimer , _BUSY_TIMEOUT_ID ) )
		{
		}
		else
		{
			//Printf("NVC Buy Wait\n");
			return ;
		}
	}
	
	if(g_pNvcItem == Null)
	{
		g_pNvcItem = Queue_Read(&g_NvcQueue);
		if(g_pNvcItem)
		{
			Nvc_SendNvcItem(g_pNvcItem);
			Queue_pop(&g_NvcQueue);
			//rt_thread_mdelay(100);		// 等待 NVC_IS_BSY()
			//Printf("NVC Start[%d-%d]\n",g_NvcQueue.readInd,g_NvcQueue.writeInd );
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
			//Printf("NVC BSY\n");
		}
		else
		{
			
			if(++g_pNvcItem->repeatCounter >= g_pNvcItem->maxRepeat)
			{
				//Nvc_RemovedItem();
				Nvc_RemovedItem_2();
				//Printf("NVC Remove\n");
			}
			else
			{
				Sif_TxByte(&g_Sif, g_pNvcItem->cmd);
				SwTimer_Start(&g_NvcTimer, 10*1000, _PALY_TIMEOUT_ID );
				//Printf("NVC Next\n");
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
