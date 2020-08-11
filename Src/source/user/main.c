/*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : main.c
 * 
 * Change Logs:
 * Date		      Author		Notes
 * 2020-07-28	  lane 	 	    first implementation
*******************************************************************************/

#include "rtthread.h"
#include "gd32f403.h"

void led_config(void);

static void led_thread_entry(void)
{
    while(1)
    {
		gpio_bit_reset(GPIOE, GPIO_PIN_11);
		rt_thread_mdelay(500);
		gpio_bit_set(GPIOE, GPIO_PIN_11);
		rt_thread_mdelay(500);
    }
}

/*!
 * \brief main function
 *		  
 * \param[in]  none
 * \param[out] none
 *
 * \return     none
 */
int main(void)
{
	rt_thread_t led_task_tid;    
	
	/* led config */
    led_config();
	

	led_task_tid= rt_thread_create("led4",/* Ïß³ÌÃû³Æ */
                            led_thread_entry, RT_NULL,
                            1024, 3, 10); //
	
	rt_thread_startup(led_task_tid);
    while(1)
	{
//		gpio_bit_reset(GPIOE, GPIO_PIN_11);
//		rt_thread_mdelay(2000);
//		gpio_bit_set(GPIOE, GPIO_PIN_11);
		rt_thread_mdelay(2000);
    }
}

/*!
    \brief      led config
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    /* enable the led clock */
    rcu_periph_clock_enable(RCU_GPIOE);
    /* configure led GPIO port */ 
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_11);
	gpio_bit_set(GPIOE, GPIO_PIN_11);
}
