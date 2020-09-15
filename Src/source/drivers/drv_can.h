/*
 * File      : drv_can.c
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-31     Hollis       for gd32f403
 */
#ifndef __CAN_H__
#define __CAN_H__

/* select CAN baudrate */
/* 1MBps */
//#define CAN_BAUDRATE  1000
/* 500kBps */
// #define CAN_BAUDRATE  500 
/* 250kBps */
#define CAN_BAUDRATE  250 
/* 125kBps */
/* #define CAN_BAUDRATE  125 */
/* 100kBps */ 
/* #define CAN_BAUDRATE  100 */
/* 50kBps */ 
/* #define CAN_BAUDRATE  50 */
/* 20kBps */ 
/* #define CAN_BAUDRATE  20 */

#define CAN_TX_ID 	0x2010
#define CAN_RX_ID	0x1020

void can0_receive_isr(void);
void can0_init(void);
void can_start(void);
//void can_run(void);
void can_sleep(void);



#endif
