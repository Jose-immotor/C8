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
 can_trasnmit_message_struct transmit_message;
 can_receive_message_struct receive_message;
 void can0_receive_isr()
{
	  /* enter interrupt */
    rt_interrupt_enter();
	 
	can_message_receive(CAN1, CAN_FIFO0, &receive_message);   
    if((CAN_RX_ID == receive_message.rx_efid) && (receive_message.rx_dlen > 0))
    {
    	//JT808_rxDataProc( receive_message.rx_data , receive_message.rx_dlen );
    	can0_receive_flag = SET; 
    }
	 /* leave interrupt */
    rt_interrupt_leave();
}
void can_start()
{
    /* enable can receive FIFO0/1 not empty interrupt */
    can_interrupt_enable(CAN1, CAN_INTEN_RFNEIE0);
    can_interrupt_enable(CAN1, CAN_INTEN_RFNEIE1);

    /* configure CAN0 NVIC */
    nvic_irq_enable(CAN1_RX0_IRQn,0,0);
//    nvic_irq_enable(CAN1_TX_IRQn,1,1);
}
void can_sleep()
{
    /* enable can receive FIFO0/1 not empty interrupt */
    can_interrupt_disable(CAN1, CAN_INTEN_RFNEIE0);
    can_interrupt_disable(CAN1, CAN_INTEN_RFNEIE1);
    //return True;
}

static void _can_init(void)
{
	/* enable CAN clock */ 
		rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_CAN0);
		rcu_periph_clock_enable(RCU_GPIOB);
	
	can_deinit(CAN1);
	
	/* configure CAN1 GPIO */
	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
	gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_12);
	/* initialize CAN1 register */
//	can_struct_para_init(CAN_INIT_STRUCT, &can_init_parameter);
	/* initialize CAN parameters 	can_init	*/
	can_init_parameter.time_triggered = DISABLE;
	can_init_parameter.auto_bus_off_recovery = DISABLE;
	can_init_parameter.auto_wake_up = DISABLE;
	can_init_parameter.auto_retrans = DISABLE;
	can_init_parameter.rec_fifo_overwrite = DISABLE;
	can_init_parameter.trans_fifo_order = DISABLE;
	can_init_parameter.working_mode = CAN_NORMAL_MODE;
	can_init_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
	can_init_parameter.time_segment_1 = CAN_BT_BS1_4TQ;
	can_init_parameter.time_segment_2 = CAN_BT_BS2_3TQ;

    /* 250KBps */
    can_init_parameter.prescaler = 18;

	/* initialize CAN */
	can_init(CAN1, &can_init_parameter);
   
    /* initialize filter */ 
    can_filter_parameter.filter_number=14;
    can_filter_parameter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter_parameter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter_parameter.filter_list_high = 0x0000;
    can_filter_parameter.filter_list_low = 0x0000;
    can_filter_parameter.filter_mask_high = 0x0000;
    can_filter_parameter.filter_mask_low = 0x0000;
    can_filter_parameter.filter_fifo_number = CAN_FIFO0;
    can_filter_parameter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter_parameter);
// can1_filter_start_bank(14);
    /* initialize transmit message *///��չ����֡   tx_efidǰ8���ֽ�Ϊ���ܷ���ַ����8���ֽ�Ϊ�Լ���ַ
 //   transmit_message.tx_sfid = 0x0010;
    transmit_message.tx_efid = CAN_TX_ID;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_EXTENDED;
    //transmit_message.tx_dlen = 2;
}

void can0_reset(void)
{
	uint32_t reset_ms = 0;
	//rcu_periph_clock_disable(RCU_AF);
    rcu_periph_clock_disable(RCU_CAN0);
	//rcu_periph_clock_enable(RCU_GPIOB);
	can_deinit(CAN0);
	reset_ms = GET_TICKS();
	while( GET_TICKS() - reset_ms < 100 );
	_can_init();
	can_start();
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
	_can_init();
}


















