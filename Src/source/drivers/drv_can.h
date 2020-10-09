/*
 * File      : drv_can.c
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-31     Hollis       for gd32f403
 */
#ifndef __CAN_H__
#define __CAN_H__

#include "gd32e10x.h"

/* select CAN baudrate */
#define CAN_BAUDRATE  250 

#define CAN_TX_ID 	0x2010
#define CAN_RX_ID	0x1020

void can0_receive_isr(void);
void hw_can_init(uint32_t can_periph);
void can_start(void);
//void can_run(void);
void can_sleep(void);



#endif
