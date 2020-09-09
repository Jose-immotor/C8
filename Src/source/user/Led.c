
#include "Common.h"
#include "led.h"
#include "drv_gpio.h"

#define RUN_LED_ON		0
#define	RUN_LED_OFF		1

static DrvIo* g_pLedIO = Null;

void led_thread_entry(void* pObj)
{
	while (1)
	{
		PortPin_Set(g_pLedIO->periph, g_pLedIO->pin, RUN_LED_ON);
		rt_thread_mdelay(200);
		PortPin_Set(g_pLedIO->periph, g_pLedIO->pin, RUN_LED_OFF);
		rt_thread_mdelay(800);
	}
}

void Led_start()
{
	rt_thread_t led_task_tid;

	led_task_tid = rt_thread_create("led4",/* Ïß³ÌÃû³Æ */
		led_thread_entry, RT_NULL,
		1024, 3, 10); //

	rt_thread_startup(led_task_tid);
}

void Led_init()
{
	const static Obj obj = {
	.name = "LED",
	.Start = Led_start,
	};

	ObjList_add(&obj);

	g_pLedIO = IO_Get(CTRL_MCU_LED);
}
