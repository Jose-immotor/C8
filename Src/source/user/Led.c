
#include "Common.h"
#include "led.h"

void led_thread_entry(void* pObj)
{
	while (1)
	{
		gpio_bit_reset(GPIOE, GPIO_PIN_11);
		rt_thread_mdelay(500);
		gpio_bit_set(GPIOE, GPIO_PIN_11);
		rt_thread_mdelay(500);
	}
}

void Led_start()
{
	rt_thread_t led_task_tid = rt_thread_create("led4",/* Ïß³ÌÃû³Æ */
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

	/* enable the led clock */
	rcu_periph_clock_enable(RCU_GPIOE);
	/* configure led GPIO port */
	gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
	gpio_bit_set(GPIOE, GPIO_PIN_11);
}
