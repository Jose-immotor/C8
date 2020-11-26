
#include "drv_systick.h"
#include "gd32e10x.h"

/**
 * This is the timer interrupt service routine.
 *
 */
void SystemClock_Handler(void)
{
	//ms ticks of system, 
	extern volatile unsigned int system_ms_tick;
	
	system_ms_tick++;
}

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SystemClock_Config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}
