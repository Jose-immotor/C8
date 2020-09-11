#ifdef OLDER_CODE

/*
 * File      : fm17522_process.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  NFC�߳����
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-16     lane         the first version
 * 2020-04-04     lane         ���ƹ���
 * ...
 */
#include <rtthread.h>
#include <string.h>
#include "drv_i2c.h"
#include "ntag_app.h"
//#include "nfc_cmd_process.h"
#include "i2c1_driver.h"

static void fm17522_GPIO_config(void)
{

//	rt_pin_mode(FM17522_ON, PIN_MODE_OUTPUT);
//	rt_pin_write(FM17522_ON, PIN_HIGH);
//	//NFC1 POWER
//	rt_pin_mode(FM17522_ON1, PIN_MODE_OUTPUT);
//	rt_pin_write(FM17522_ON1, PIN_HIGH);
	rcu_periph_clock_enable(RCU_GPIOE);
	//NFC0
	/*NPD���ƽ�,����оƬ��NPD�͵�ƽʱ�򲻹���*/
	gpio_init(FM17522_NPD_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FM17522_NPD_PIN);
	FM17522_NPD_LOW;
	//NFC0 POWER	
	gpio_init(FM17522_POWER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FM17522_POWER_PIN);
	FM17522_POWER_ON;

//    /* �� ��0�� �� Ϊ �� �� ģ ʽ */
//    rt_pin_mode(FM17522_IRQ, PIN_MODE_INPUT);
//    /* �� �� �� �� �� �� �� �� ģ ʽ�� �� �� �� �� �� Ϊ beep_on */
////    rt_pin_attach_irq(FM17522_IRQ, PIN_IRQ_MODE_RISING_FALLING, fm17522_nfc_irq_handler, RT_NULL);
//    /* ʹ �� �� �� */
////    rt_pin_irq_enable(FM17522_IRQ, PIN_IRQ_ENABLE);
//	//NFC1
//	rt_pin_mode(FM17522_NPD1, PIN_MODE_OUTPUT);
//	rt_pin_write(FM17522_NPD1, PIN_LOW);
	rt_thread_mdelay(500);
}
/**
 * @brief fm17522_param_init
 * @param[in] none
 * @param[out] none
 * @param 
 * @return 
 * 
 * @attention 
 */
void thread_fm17522_entry(void* parameter)
{
//    iic_dev_init();
	rt_hw_i2c_init(NFC_I2C);
	fm17522_GPIO_config();
    for(;;)
    {
		NTAG_task_process();
		//�ӳ�
		rt_thread_mdelay(200);
    }
}

//NFC���ּ�ʱ��
static void nfc_ms_timer_cb(void* parameter)
{
	Pcd_Comm_timer_cb();//NFC timeout
	NTAG_task_timer_cb();//���NFCͨѶ��ʱ�����û��ͨѶ10������³�ʼ��
}

struct rt_thread thread_nfc;
unsigned char thread_nfc_stack[2048];
rt_timer_t nfc_ms_handler;
rt_sem_t nfc_sem = RT_NULL;
//�̳߳�ʼ��
static int app_nfc_init(void)
{
	rt_err_t res;
	
	nfc_sem = rt_sem_create("nfcsem", 0, RT_IPC_FLAG_FIFO);
    if (nfc_sem == RT_NULL)
    {
        rt_kprintf("create nfc semaphore failed.\n");
        return -1;
    }
	
	res=rt_thread_init(&thread_nfc,"nfc",thread_fm17522_entry,RT_NULL,&thread_nfc_stack[0],
    sizeof(thread_nfc_stack), 2, 10);	
    if (res == RT_EOK) /* �������߳̿��ƿ飬��������߳� */
        rt_thread_startup(&thread_nfc);
	else
		rt_kprintf("\r\n!!create thread fm17522 failed!\r\n");
	
	nfc_ms_handler = rt_timer_create("nfc_ms_timer", nfc_ms_timer_cb, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);
    if (nfc_ms_handler != RT_NULL) 
        rt_timer_start(nfc_ms_handler);
	
    return 0;
}
INIT_APP_EXPORT(app_nfc_init);

#endif
