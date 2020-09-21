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

#include <stdint.h>
#include "queue.h"

#define SHELL_USART0
#define RS485_UART4		USART4

//485接收数组buff长度
#define RX_BUFF_SIZE          256

uint32_t usart0_get_byte(uint8_t *data);
uint32_t usart0_put_byte(uint8_t data);
uint8_t usart0_send(void *buffer, uint8_t cnt);

char rs485_getchar(void);

#endif
