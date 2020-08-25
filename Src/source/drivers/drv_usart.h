/*
 * File      : drv_usart.h
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
 */

#ifndef __USART_H__
#define __USART_H__

#include "queue.h"

#define SHELL_USART0

//485发送数组buff长度
#define TX_BUFF_SIZE          256
//485接收数组buff长度
#define RX_BUFF_SIZE          256



typedef struct
{
	uint8_t rx_buf[RX_BUFF_SIZE];
	//uint8_t tx_buf[TX_BUFF_SIZE];
	Queue rx_fifo;
	//Queue tx_fifo;

	uint8_t rx_flag;
	uint32_t (*get_byte)(uint8_t*);
	uint32_t (*put_byte)(uint8_t data);
	void (*tx_enable)(Bool enable);
	Bool (*tx_busy)(void);
}usart_t;

extern usart_t __g_usart0;

uint32_t usart0_get_byte(uint8_t *data);
uint8_t usart0_send(void *buffer, uint8_t cnt);

#endif
