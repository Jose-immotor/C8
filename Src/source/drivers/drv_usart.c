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

uint8_t Shell_rx_buf[RX_BUFF_SIZE];
Queue QuenueShellRx;
void Shell_rxCmd(const char* str);
void uart0_isr(void)
{
	rt_interrupt_enter();
	if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET)
    {
		uint8_t data = usart_data_receive(USART0);
		if(Queue_writeByte(&QuenueShellRx, data))
		{
		}
		else
		{
			//overflow
		}
    }
	rt_interrupt_leave();

}

char rt_hw_console_getchar(void)
{
	#if 1
	uint8_t* data;
	
	data = (uint8_t*)Queue_pop(&QuenueShellRx);
	if (data)
	{
		return *data;
	}
	else
	{
		return (char)-1;
	}
	#else
//	//使用查询方式
//	int ch = -1;
//	
//	if (usart_flag_get(USART0, USART_FLAG_RBNE) != RESET)
//    {
//        ch = usart_data_receive(USART0);
//    }
//	return ch;
	#endif
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
	while(RESET == usart_flag_get(USART0, USART_FLAG_TC));
    return sta;   
}
#if 1
//Queue QuenueRs485Rx;
//extern CirBuff gRs485rxBuf;
//extern CirBuff gTx485txBuf;
extern void usart_push_data( uint8_t data );

void uart4_isr(void)
{
	rt_interrupt_enter();
	if(usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE) != RESET)
    {
		uint8_t data = usart_data_receive(UART4);
		usart_push_data( data );
		/*
		if(Queue_writeByte(&QuenueRs485Rx, data))
		{
		}
		else
		{
			//overflow
		}
		*/
    }
	rt_interrupt_leave();
}
/*
char rs485_getchar(void)
{
	uint8_t* data;
	
	data = (uint8_t*)Queue_pop(&QuenueRs485Rx);
	if (data)
	{
		return *data;
	}
	else
	{
		return (char)-1;
	}
}
*/

uint32_t uart4_put_byte(uint8_t data)
{    
    uint32_t sta;

	usart_data_transmit(UART4, data);
	while(RESET == usart_flag_get(UART4, USART_FLAG_TC));
    return sta;   
}
#endif
void gd32_hw_usart_deinit(void)
{
#ifdef SHELL_USART0	
	rcu_periph_clock_disable(RCU_USART0);
	// A9 A10
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
#endif

#ifdef RS485_UART4
	rcu_periph_clock_disable(RCU_UART4);
	// C12 D12
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
#endif

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
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	nvic_irq_enable(USART0_IRQn, 0, 0);
	
	Queue_init(&QuenueShellRx, Shell_rx_buf, 1, RX_BUFF_SIZE);
    usart_enable(USART0);
#endif
#ifdef RS485_UART4
	
	
	//uint8_t Rs485_rx_buf[RX_BUFF_SIZE];
	
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_UART4);
    rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_AF);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    
    /* USART configure */
    usart_deinit(UART4);
    usart_baudrate_set(UART4, 9600U);
	usart_word_length_set(UART4, USART_WL_8BIT);
	usart_stop_bit_set(UART4, USART_STB_1BIT);
	usart_parity_config(UART4, USART_PM_NONE);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
	usart_interrupt_enable(UART4, USART_INT_RBNE);
	//Queue_init(&QuenueRs485Rx, Rs485_rx_buf, 1, RX_BUFF_SIZE);
    usart_enable(UART4);
	
	nvic_irq_enable(UART4_IRQn, 0, 0);
	
#endif
    return 0;
}
INIT_BOARD_EXPORT(gd32_hw_usart_init);
