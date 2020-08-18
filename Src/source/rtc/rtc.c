/*
 * File      : rtc.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
#include "rtc.h"
#include "main.h"
#include "date.h"
#include <string.h>
#include "common.h"
S_RTC_TIME_DATA_T sInitTime;

/*!
    \brief      configure the RTC
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_configuration(void)
{
	/* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();
    /* reset backup domain */
    bkp_deinit();
//	nvic_irq_enable(RTC_ALARM_IRQn, 1U, 0U);
	nvic_irq_enable(RTC_IRQn, 0U, 0U);
	exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	
#ifdef USE_RTC_LXTAL	
    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
#else
    /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_IRC40K);
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
#endif	
    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);
    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* set RTC prescaler: set RTC period to 1s */
	rtc_prescaler_set(RTC_FCR_REFERENCE);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
} 
/**
 * @brief LocalTimeInit
 * @param[in] none
 * @param[out] none
 * @param 
 * @return 
 * 
 * @attention 
 */
void LocalTimeInit(void)
{
    static struct rtc_time systime;
	/* Time Setting */
    sInitTime.u32Year       = 2020;
    sInitTime.u32Month      = 1;
    sInitTime.u32Day        = 1;
    sInitTime.u32Hour       = 12;
    sInitTime.u32Minute     = 0;
    sInitTime.u32Second     = 0;
    sInitTime.u32DayOfWeek  = RTC_WEDNESDAY;
    sInitTime.u32TimeScale  = RTC_CLOCK_24;
	sInitTime.u32AmPm       = RTC_AM;
	
	memcpy(&systime,&sInitTime,sizeof(struct rtc_time));

	rtc_configuration();	
    /* change the current time */
    rtc_counter_set(mktimev(&systime));
	/* Wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
}
/**
 * @brief LocalTimeInit
 * @param[in] cnt �����
 * @param[out] none
 * @param 
 * @return 
 * 
 * @attention 
 */

void LocalTimeSync(S_RTC_TIME_DATA_T* time )
{
	static struct rtc_time systime;
	static uint32_t timedata=0;
	
	/* Time Setting */
	Printf("DataTime sync:");
	DateTime_dump(time);
	
	sInitTime.u32Year       = time->u32Year;
	sInitTime.u32Month      = time->u32Month;
	sInitTime.u32Day        = time->u32Day;
	sInitTime.u32Hour       = time->u32Hour;
	sInitTime.u32Minute     = time->u32Minute;
	sInitTime.u32Second     = time->u32Second;
	
	
	memcpy(&systime,&sInitTime,sizeof(struct rtc_time));
	
	timedata = mktimev(&systime);
	
    /* change the current time */
    rtc_counter_set(timedata);
    rtc_lwoff_wait();	
}

/**
 *  @brief    Read current date/time from RTC setting
 *
 *  @param[out]    sPt \n
 *                     Specify the time property and current time. It includes: \n
 *                     u32Year: Year value                                      \n
 *                     u32Month: Month value                                    \n
 *                     u32Day: Day value                                        \n
 *                     u32DayOfWeek: Day of week                                \n
 *                     u32Hour: Hour value                                      \n
 *                     u32Minute: Minute value                                  \n
 *                     u32Second: Second value                                  \n
 *                     u32TimeScale: \ref RTC_CLOCK_12 / \ref RTC_CLOCK_24          \n
 *                     u8AmPm: \ref RTC_AM / \ref RTC_PM                            \n
 *
 *  @return   None
 *
 */
void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt)
{
	uint32_t timevar=0;
	static struct rtc_time systime;


	timevar = rtc_counter_get();
	
	to_tm(timevar, &systime);
	
	memcpy(&sInitTime,&systime,sizeof(struct rtc_time));
	
    sPt->u32TimeScale = sInitTime.u32TimeScale;    /* 12/24-hour */
    sPt->u32DayOfWeek = sInitTime.u32DayOfWeek;          /* Day of week */
    sPt->u32Year      = sInitTime.u32Year;
    sPt->u32Month     = sInitTime.u32Month;
    sPt->u32Day       = sInitTime.u32Day;	
	
    if (sInitTime.u32TimeScale == RTC_CLOCK_12)  /* Compute12/24 hour */
    {
		if(sInitTime.u32Hour >= 13)
		{
		    sPt->u32AmPm = RTC_PM;
			sPt->u32Hour = sInitTime.u32Hour - 12;
		}
		else
		{
		    sPt->u32AmPm = RTC_AM;
			sPt->u32Hour = sInitTime.u32Hour;
		}
		sPt->u32Minute   = sInitTime.u32Minute;
		sPt->u32Second   = sInitTime.u32Second;
    }
	else 
	{
        sPt->u32Hour     = sInitTime.u32Hour;
		sPt->u32Minute   = sInitTime.u32Minute;
		sPt->u32Second   = sInitTime.u32Second;
    }
}

//RTC��ʱ����
void RTC_TimerStart(uint32_t second)
{
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);	
	/* Enable RTC Clock */
	rcu_periph_clock_enable(RCU_RTC);
	/* Wait for RTC registers synchronization */
	rtc_register_sync_wait();

	rtc_lwoff_wait();
	rtc_alarm_config(rtc_counter_get()+second);
	/* Wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
		
	rtc_interrupt_enable(RTC_INT_ALARM);
	rtc_lwoff_wait();
}

/**
  * @brief  This function handles RTC Alarm interrupt request.
  * @param  None
  * @retval None
  */
void RTC_Alarm_IRQHandler(void)
{
	rt_interrupt_enter();
	if(rtc_flag_get(RTC_FLAG_ALARM) != RESET)
	{
//		rtc_flag_clear(RTC_FLAG_ALARM);        ///<Clear EXTI line17 pending bit
//		if(pmu_flag_get(PMU_FLAG_WAKEUP) != RESET) ///<Check if the Wake-Up flag is set
//		{
//			pmu_flag_clear(PMU_FLAG_WAKEUP);             ///<Clear Wake Up flag
//		}
//		rtc_lwoff_wait();		                       ///<Wait until last write operation on RTC registers has finished
		rtc_flag_clear(RTC_FLAG_ALARM);        	///<Clear RTC Alarm interrupt pending bit
		rtc_lwoff_wait();		                       ///<Wait until last write operation on RTC registers has finished
		
//		PostMsg(MSG_RTC_TIMEOUT);
		if(g_isPowerDown)	
		{
			SetWakeUpType(WAKEUP_RTC);
		}
	}
	rt_interrupt_leave();
}
/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
	rt_interrupt_enter();
	if (rtc_flag_get(RTC_FLAG_ALARM) != RESET)
	{
		SetWakeUpType(WAKEUP_RTC);
		/* Clear Interrupt pending bit */
		rtc_flag_clear(RTC_FLAG_ALARM);
	}
	rt_interrupt_leave();
}

