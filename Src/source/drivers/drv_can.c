/*
 * File      : drv_can.c
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-31     Hollis       for gd32f403
 */
#include "drv_can.h"
#include <string.h>
#include "gd32e10x.h"
#include "Common.h"

 FlagStatus can0_receive_flag;
 can_parameter_struct can_init_parameter;
 can_filter_parameter_struct can_filter_parameter;
 can_trasnmit_message_struct transmit_message;
 can_receive_message_struct receive_message;
 void can0_receive_isr()
 {
	  /* enter interrupt */
    rt_interrupt_enter();
	 
	  can_message_receive(CAN0, CAN_FIFO0, &receive_message);   
    //if((0x300>>1 == receive_message.rx_sfid)&&(CAN_FF_STANDARD == receive_message.rx_ff)&&(2 == receive_message.rx_dlen))
    if( (0x1020 == receive_message.rx_efid) && (receive_message.rx_dlen > 0) )
    can0_receive_flag = SET; 		   
	 
	 /* leave interrupt */
    rt_interrupt_leave();
 }
void can_thread_entry(void* pObj)
{
	while(1)
	{
		//Printf("\r\n can0 transmit data:%x,%x", transmit_message.tx_data[0], transmit_message.tx_data[1]);
		/* transmit message */
		//can_message_transmit(CAN0, &transmit_message);

//		if(can0_receive_flag == SET)
//		{
//			can0_receive_flag = RESET;
//			printf("\r\n can0 receive \r\n Data:");
//			for(uint8 vl_rx_cnt=0; vl_rx_cnt < receive_message.rx_dlen;vl_rx_cnt++)
//			{
//				printf("%x ", receive_message.rx_data[vl_rx_cnt]);
//			}
//			
//		}
		rt_thread_mdelay(1);		
	}
}
void can_start()
{
    /* enable can receive FIFO0/1 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0);
    can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE1);

    /* configure CAN0 NVIC */
    nvic_irq_enable(CAN0_RX0_IRQn,0,0);
    nvic_irq_enable(CAN0_TX_IRQn,1,1);
//    rt_thread_t can_task_tid= rt_thread_create("can_send_rec",/* can收发 */
//                        can_thread_entry, RT_NULL,
//                        1024,2, 10); //
//    rt_thread_startup(can_task_tid);
}
void can_sleep()
{
    /* enable can receive FIFO0/1 not empty interrupt */
    can_interrupt_disable(CAN0, CAN_INTEN_RFNEIE0);
    can_interrupt_disable(CAN0, CAN_INTEN_RFNEIE1);
    //return True;
}
void can0_init(void)
{
//    const static Obj obj = { "can0", can_start, Null, Null };
//    bjList_add(&obj);
    const static Obj obj = {
	.name = "CAN",
	.Start = can_start,
	};
    ObjList_add(&obj);
		/* enable CAN clock */ 
		rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_CAN0);
		rcu_periph_clock_enable(RCU_GPIOB);
	
	/* configure CAN0 GPIO */
    gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_9);
    gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
	gpio_pin_remap_config(GPIO_CAN0_PARTIAL_REMAP,ENABLE);	
	/* initialize CAN0 register */
    can_deinit(CAN0);
	/* initialize CAN parameters 	can_init	*/
	can_init_parameter.time_triggered = DISABLE;
	can_init_parameter.auto_bus_off_recovery = DISABLE;
	can_init_parameter.auto_wake_up = DISABLE;
	can_init_parameter.auto_retrans = DISABLE;
	can_init_parameter.rec_fifo_overwrite = DISABLE;
	can_init_parameter.trans_fifo_order = DISABLE;
	can_init_parameter.working_mode = CAN_NORMAL_MODE;
	can_init_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
	can_init_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
	can_init_parameter.time_segment_2 = CAN_BT_BS2_5TQ;
    /* 1MBps */
#if CAN_BAUDRATE == 1000
    can_init_parameter.prescaler = 7;
    /* 500KBps */
#elif CAN_BAUDRATE == 500
    can_init_parameter.prescaler = 14;
    /* 250KBps */
#elif CAN_BAUDRATE == 250
    can_init_parameter.prescaler = 28;
    /* 125KBps */
#elif CAN_BAUDRATE == 125
    can_init_parameter.prescaler = 56;
    /* 100KBps */
#elif  CAN_BAUDRATE == 100
    can_init_parameter.prescaler = 70;
    /* 50KBps */
#elif  CAN_BAUDRATE == 50
    can_init_parameter.prescaler = 140;
    /* 20KBps */
#elif  CAN_BAUDRATE == 20
    can_init_parameter.prescaler = 350;
//#else
//    #error "please select list can baudrate in private defines in main.c "
#endif  
    /* initialize CAN */
    can_init(CAN0, &can_init_parameter);
		
		    
    /* initialize filter */ 
    can_filter_parameter.filter_number=0;
    can_filter_parameter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter_parameter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter_parameter.filter_list_high = 0x0000;
    can_filter_parameter.filter_list_low = 0x0000;
    can_filter_parameter.filter_mask_high = 0x0000;
    can_filter_parameter.filter_mask_low = 0x0000;
    can_filter_parameter.filter_fifo_number = CAN_FIFO0;
    can_filter_parameter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter_parameter);

    /* initialize transmit message *///拓展数据帧   tx_efid前8个字节为接受方地址，后8个字节为自己地址
    transmit_message.tx_sfid = 0x0010;
    transmit_message.tx_efid = 0x2010;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_EXTENDED;
    //transmit_message.tx_dlen = 2;
}


















