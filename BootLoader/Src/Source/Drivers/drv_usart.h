
#ifndef __USART_H__
#define __USART_H__

#include <stdint.h>

//485接收数组buff长度
#define RX_BUFF_SIZE          256

uint32_t usart0_get_byte(uint8_t *data);
uint32_t usart0_put_byte(uint8_t data);
uint8_t usart0_send(void *buffer, uint8_t cnt);
int gd32_hw_usart_init(void);


#endif
