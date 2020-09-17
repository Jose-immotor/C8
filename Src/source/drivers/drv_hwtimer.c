/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-16     Leo          first version
 */
 
#include <board.h>
#include "drv_hwtimer.h"

//enum
//{
//#ifdef BSP_USING_HWTMR1
//    TMR1_INDEX,
//#endif 
//  
//#ifdef BSP_USING_HWTMR2
//    TMR2_INDEX,
//#endif 
//  
//#ifdef BSP_USING_HWTMR3
//    TMR3_INDEX,
//#endif 

//#ifdef BSP_USING_HWTMR4
//    TMR4_INDEX,
//#endif 

//#ifdef BSP_USING_HWTMR5
//    TMR5_INDEX,
//#endif  

//#ifdef BSP_USING_HWTMR6
//    TMR6_INDEX,
//#endif  

//#ifdef BSP_USING_HWTMR7
//    TMR7_INDEX,
//#endif 

//#ifdef BSP_USING_HW_TMR8
//    TMR8_INDEX,
//#endif  

//#ifdef BSP_USING_HWTMR9
//    TMR9_INDEX,
//#endif

//#ifdef BSP_USING_HWTMR10
//    TMR10_INDEX,
//#endif 

//#ifdef BSP_USING_HWTMR11
//    TMR11_INDEX,
//#endif  

//#ifdef BSP_USING_HWTMR12
//    TMR12_INDEX,
//#endif

//#ifdef BSP_USING_HWTMR13
//    TMR13_INDEX,
//#endif 

//#ifdef BSP_USING_HWTMR14
//    TMR14_INDEX,
//#endif  

//#ifdef BSP_USING_HWTMR15
//    TMR15_INDEX,
//#endif
//};

//struct at32_hwtimer
//{
//    rt_hwtimer_t  time_device;
//    TMR_Type*     tim_handle;
//    IRQn_Type     tim_irqn;
//    char          *name;
//};

//static struct at32_hwtimer at32_hwtimer_obj[] =
//{
//#ifdef BSP_USING_HWTMR1
//    TMR1_CONFIG,
//#endif  

//#ifdef BSP_USING_HWTMR2
//    TMR2_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR3
//    TMR3_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR4
//    TMR4_CONFIG,
//#endif

//#ifdef BSP_USING_HWTMR5
//    TMR5_CONFIG,
//#endif 

//#ifdef BSP_USING_HWTMR6
//    TMR6_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR7
//    TMR7_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR8
//    TMR8_CONFIG,
//#endif

//#ifdef BSP_USING_HWTMR9
//    TMR9_CONFIG,
//#endif 

//#ifdef BSP_USING_HWTMR10
//    TMR10_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR11
//    TMR11_CONFIG,
//#endif
//  
//#ifdef BSP_USING_HWTMR12
//    TMR12_CONFIG,
//#endif

//#ifdef BSP_USING_HWTMR13
//    TMR13_CONFIG,
//#endif

//#ifdef BSP_USING_HWTMR14
//    TMR14_CONFIG,
//#endif

//#ifdef BSP_USING_HWTMR15
//    TMR15_CONFIG,
//#endif
//};

//static void at32_timer_init(struct rt_hwtimer_device *timer, rt_uint32_t state)
//{
//    RCC_ClockType RCC_ClockStruct;
//    TMR_TimerBaseInitType TMR_TMReBaseStructure;
//    NVIC_InitType NVIC_InitStructure;
//    uint32_t prescaler_value = 0;
//    TMR_Type *tim = RT_NULL;
//    struct at32_hwtimer *tim_device = RT_NULL;

//    RT_ASSERT(timer != RT_NULL);
//    if (state)
//    {
//        tim = (TMR_Type *)timer->parent.user_data;
//        tim_device = (struct at32_hwtimer *)timer;
//        
//        /* timer clock enable */
//        at32_msp_hwtmr_init(tim);
//        
//        /* timer init */
//        RCC_GetClocksFreq(&RCC_ClockStruct);
//        /* Set timer clock is 1Mhz */
//        prescaler_value = (uint32_t)(RCC_ClockStruct.SYSCLK_Freq / 10000) - 1;
//        
//        TMR_TMReBaseStructure.TMR_Period = 10000 - 1;
//        TMR_TMReBaseStructure.TMR_DIV = prescaler_value;
//        TMR_TMReBaseStructure.TMR_ClockDivision = TMR_CKD_DIV1;
//        TMR_TMReBaseStructure.TMR_RepetitionCounter = 0;
//        
//        if (timer->info->cntmode == HWTIMER_CNTMODE_UP)
//        {
//            TMR_TMReBaseStructure.TMR_CounterMode = TMR_CounterDIR_Up;
//        }
//        else
//        {
//            TMR_TMReBaseStructure.TMR_CounterMode = TMR_CounterDIR_Down;
//        }
//        
//        TMR_TimeBaseInit(tim, &TMR_TMReBaseStructure);
//        
//        /* Enable the TMRx global Interrupt */
//        NVIC_InitStructure.NVIC_IRQChannel = tim_device->tim_irqn;
//        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//        NVIC_Init(&NVIC_InitStructure);
//        
//        TMR_INTConfig(tim, TMR_INT_Overflow ,ENABLE);
//        TMR_ClearITPendingBit(tim, TMR_INT_Overflow);
//        
////        LOG_D("%s init success", tim_device->name);
//    }      
//}

//static rt_err_t at32_timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
//{
//    rt_err_t result = RT_EOK;
//    TMR_Type *tim = RT_NULL;

//    RT_ASSERT(timer != RT_NULL);

//    tim = (TMR_Type *)timer->parent.user_data;
//    
//    /* set tim cnt */
//    TMR_SetCounter(tim, 0);
//    /* set tim arr */
//    TMR_SetAutoreload(tim, t - 1);
//    if (opmode == HWTIMER_MODE_ONESHOT)
//    {
//        /* set timer to single mode */
//        TMR_SelectOnePulseMode(tim, TMR_OPMode_Once);
//    }
//    else
//    {
//        TMR_SelectOnePulseMode(tim, TMR_OPMode_Repetitive);
//    }
//    
//    /* start timer */
//    TMR_Cmd(tim, ENABLE);
//    
//    return result;
//}

//static void at32_timer_stop(rt_hwtimer_t *timer)
//{
//    TMR_Type *tim = RT_NULL;

//    RT_ASSERT(timer != RT_NULL);

//    tim = (TMR_Type *)timer->parent.user_data;
//    
//    /* stop timer */
//    TMR_Cmd(tim, ENABLE);
//    /* set tim cnt */
//    TMR_SetCounter(tim, 0);
//}

//static rt_uint32_t at32_timer_counter_get(rt_hwtimer_t *timer)
//{
//    TMR_Type *tim = RT_NULL;

//    RT_ASSERT(timer != RT_NULL);

//    tim = (TMR_Type *)timer->parent.user_data;
//    
//    return tim->CNT;
//}

int rt_hw_hwtimer_start(void)
{

}

int rt_hw_hwtimer_init(void)
{
	uint32_t prescaler_value = 0;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);
    timer_deinit(TIMER3);
	
	/* Set timer clock is 1Mhz */
	prescaler_value= rcu_clock_freq_get(CK_SYS);
	prescaler_value= prescaler_value/1000000 -1;
    prescaler_value = prescaler_value;

    /* TIMER0 configuration */
    timer_initpara.prescaler         = prescaler_value;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 100;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3,&timer_initpara);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER3);
	timer_update_event_enable(TIMER3);
	
	nvic_irq_enable(TIMER3_IRQn, 0, 2);
	timer_interrupt_enable(TIMER3,TIMER_INT_UP);
	
	timer_disable(TIMER3);
    return RT_EOK;
}

