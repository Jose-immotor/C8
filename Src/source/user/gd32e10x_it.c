/*!
    \file  gd32e10x_it.c
    \brief interrupt service routines
    
    \version 2018-03-26, V1.0.0, demo for GD32E10x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32e10x_it.h"
#include "systick.h"

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void HardFault_Handler(void)
//{
//    /* if Hard Fault exception occurs, go to infinite loop */
//    while (1){
//    }
//}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SysTick_Handler(void)
//{
//    delay_decrement();
//}
/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void uart0_isr(void);
void USART0_IRQHandler(void)
{
	uart0_isr();
}

void uart4_isr(void);
void UART4_IRQHandler(void)
{
	uart4_isr();
}

extern void over_temp_irq(void);
void EXTI1_IRQHandler(void)
{
	if(SET == exti_interrupt_flag_get(EXTI_1)){
        exti_interrupt_flag_clear(EXTI_1);
		over_temp_irq();
    }
}

extern void gprs_insert(void);
void EXTI2_IRQHandler(void)
{
	if(SET == exti_interrupt_flag_get(EXTI_2)){
        exti_interrupt_flag_clear(EXTI_2);
		gprs_insert();
    }
}

void take_apart_irq(void);
void EXTI3_IRQHandler(void)
{
	if(SET == exti_interrupt_flag_get(EXTI_3)){
        exti_interrupt_flag_clear(EXTI_3);
		take_apart_irq();
    }
}

extern void Mcu_CabinLockIsr();
extern void Gyro_Isr();
extern void bat_insert(void);
void EXTI5_9_IRQHandler(void)
{
	if(SET == exti_interrupt_flag_get(EXTI_6)){
        exti_interrupt_flag_clear(EXTI_6);
		Mcu_CabinLockIsr();
    }
	if(SET == exti_interrupt_flag_get(EXTI_8)){
        exti_interrupt_flag_clear(EXTI_8);
		Gyro_Isr();
    }
//	if(SET == exti_interrupt_flag_get(EXTI_9)){
//        exti_interrupt_flag_clear(EXTI_9);
//		bat_insert();
//    }
}

void EXTI10_15_IRQHandler(void)
{
    if(SET == exti_interrupt_flag_get(EXTI_13)){
        exti_interrupt_flag_clear(EXTI_13);
    }
}

extern void can1_receive_isr(void);
void CAN1_RX1_IRQHandler(void)
{
    /* check the receive message */
		can1_receive_isr(); 
}

extern void can1_receive_isr(void);
void CAN1_RX0_IRQHandler(void)
{
    /* check the receive message */
		can1_receive_isr(); 
}

