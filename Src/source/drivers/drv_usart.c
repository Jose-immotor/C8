/*
 * File      : drv_usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2018-04-19     misonyo      Porting for gd32f30x
 */

//#include <rthw.h>
#include <rtthread.h>
#include "gd32e10x.h"
#include "drv_usart.h"

char rt_hw_console_getchar(void)
{
	//使用查询方式
	int ch = -1;
	
	if (usart_flag_get(USART0, USART_FLAG_RBNE) != RESET)
    {
        ch = usart_data_receive(USART0);
    }
	return ch;
}

/*!
 * @brief  handle slave rs485 communication
 *		  
 * \param  None
 *
 * \return None
 */
uint32_t usart0_put_byte(uint8_t data)
{    
    uint32_t sta;

	usart_data_transmit(USART0, data);
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return sta;   
}

/*!
 * \brief usart0作为调试串口，只使用接收中断.使用发送中断会出现Hard Fault
 *		  
 * \param[in]  none
 * \param[out] none
 *
 * \return     none
 */
int gd32_hw_usart_init(void)
{	
#ifdef SHELL_USART0
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    
    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
	usart_word_length_set(USART0, USART_WL_8BIT);
	usart_stop_bit_set(USART0, USART_STB_1BIT);
	usart_parity_config(USART0, USART_PM_NONE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
#endif	

    return 0;
}
INIT_BOARD_EXPORT(gd32_hw_usart_init);
