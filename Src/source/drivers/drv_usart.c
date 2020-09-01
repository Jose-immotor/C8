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
#include "gd32f403.h"
#include "drv_usart.h"
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

usart_t __g_usart0;

#define PRINTF_BUF_SIZE 256

void usart0_isr(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
	
	if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET)
    {
		uint8_t data = usart_data_receive(USART0);
		if(Queue_writeByte(&__g_usart0.rx_fifo, data))
		{
		}
		else
		{
			//overflow
		}
    }
    /* leave interrupt */
    rt_interrupt_leave();
}

char rt_hw_console_getchar(void)
{
	uint8_t* data;
	
	data = (uint8_t*)Queue_pop(&__g_usart0.rx_fifo);
	if (data)
	{
		return *data;
	}
	else
	{
		return -1;
	}
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

#if 0
//uint8_t usart0_send(void *buffer, uint8_t cnt)
//{
//	uint8_t i;
//	uint8_t tx_buf[TX_BUFF_SIZE];
//	
//	memcpy(tx_buf, buffer, cnt);
//	for(i=0; i< cnt; i++)
//	{	
//		usart0_put_byte(tx_buf[i]);
//	}

//	return 0;
//}

void _puts (const char *s)
{
    for(uint8_t i=0; i<(TX_BUFF_SIZE-1); i++)
    {
        if(s[i]!='\0')
        {
            usart0_put_byte(s[i]);
        }
        else
        {
            break;
        }
    }
}

int _Printf(const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	char g_Pfbuffer[PRINTF_BUF_SIZE];

	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	va_start(ptr, lpszFormat);
	nLen = vsnprintf(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	
	va_end(ptr);
	
	_puts(g_Pfbuffer);

	return nLen;
}

#endif

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
	usart_t *usart;
	
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
	usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_enable(USART0);
	
	nvic_irq_enable(USART0_IRQn, 0, 0);
	
	usart = &__g_usart0;
		
	memset(usart, 0, sizeof(usart_t));

    Queue_init(&usart->rx_fifo, usart->rx_buf, 1, RX_BUFF_SIZE);
#endif	

    return 0;
}
INIT_BOARD_EXPORT(gd32_hw_usart_init);
