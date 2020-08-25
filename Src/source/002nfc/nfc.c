#ifdef OLDER_CODE

/*
 * File      : fm17522_process.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  NFC线程入口
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-16     lane         the first version
 * 2020-04-04     lane         完善功能
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
	/*NPD控制脚,读卡芯片在NPD低电平时候不工作*/
	gpio_init(FM17522_NPD_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FM17522_NPD_PIN);
	FM17522_NPD_LOW;
	//NFC0 POWER	
	gpio_init(FM17522_POWER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FM17522_POWER_PIN);
	FM17522_POWER_ON;

//    /* 按 键0引 脚 为 输 入 模 式 */
//    rt_pin_mode(FM17522_IRQ, PIN_MODE_INPUT);
//    /* 绑 定 中 断 ， 上 升 沿 模 式， 回 调 函 数 名 为 beep_on */
////    rt_pin_attach_irq(FM17522_IRQ, PIN_IRQ_MODE_RISING_FALLING, fm17522_nfc_irq_handler, RT_NULL);
//    /* 使 能 中 断 */
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
		//延迟
		rt_thread_mdelay(200);
    }
}

//NFC部分计时器
static void nfc_ms_timer_cb(void* parameter)
{
	Pcd_Comm_timer_cb();//NFC timeout
	NTAG_task_timer_cb();//检测NFC通讯计时，如果没有通讯10秒后，重新初始化
}

struct rt_thread thread_nfc;
unsigned char thread_nfc_stack[2048];
rt_timer_t nfc_ms_handler;
rt_sem_t nfc_sem = RT_NULL;
//线程初始化
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
    if (res == RT_EOK) /* 如果获得线程控制块，启动这个线程 */
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
