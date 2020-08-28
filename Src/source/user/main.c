/*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : main.c
 * 
 * Change Logs:
 * Date		      Author		Notes
 * 2020-07-28	  lane 	 	    first implementation
 * 2020-08-27	  Allen	 	    modified
*******************************************************************************/

#include "rtc.h"
#include "pms.h"
#include "nvds.h"
#include "Common.h"
#include "led.h"
#include "JT808.h"
#include "Obj.h"

volatile bool g_isPowerDown = False;//���߱�־��False-û�����ߣ�True-����

//WakeupType g_WakeupType;
//WakeupType GetWakeUpType()
//{
//	return g_WakeupType;
//}
//void SetWakeUpType(WakeupType type)
//{
//	g_WakeupType = type;
//}

//������ʱ��λ���ܣ���Ҫ������ӦһЩ�ⲿ���ִ�к���ҪӦ��Ȼ��ִ��MCU��λ����
uint32 g_ResetMs = 0;
static uint32_t g_ResetInitMs = 0;
//static Bool g_isPowerOff = False;
//void ResetDelay(MCURST reason, uint32 ms);
void DelayPowerOff(uint32 ms)
{
//	ResetDelay(MCURST_18650_POWER_OFF, ms);
//	g_isPowerOff = True;
}
void ResetStop()
{
	g_ResetMs = 0;
//	Fsm_SetActiveFlag(AF_MCU_RESET, False);
}
//void ResetDelay(MCURST reason, uint32 ms)
//{
////	Printf("Mcu %s after %d ms\n", (g_pPms->m_isTestBat) ? "shut down":"reset", ms);
//	
//	g_SysCfg.resetReason = reason;
//	g_isPowerOff = False;
//	g_ResetInitMs = GET_TICKS();
//	g_ResetMs = ms;
//	if(ms)
//	{
////		Fsm_SetActiveFlag(AF_MCU_RESET, True);
//	}
//}
void Boot(Bool isFromAppRom)
{
	Printf("Mcu reset\n");

	//��λ֮ǰ����
//	Nvds_Write_Setting();
//	Nvds_Write_SysCfg();
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //�ر������ж�
	NVIC_SystemReset();
}

/*!
 * \brief ��������ʱIO��ƽ�����û����ж�
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
void Enter_PowerDown()
{	
	RTC_TimerStart(24*60*60);
//	//485 power
//	POWER_3V3_485_OFF;
//	//NFC power
//	rt_pin_write(FM17522_ON, PIN_LOW);
//	rt_pin_write(FM17522_ON1, PIN_LOW);
//	ADC_Ctrl(ADC1, DISABLE);
//	//SIMģ�黽��MCU
//	bsp_sim_rx_int_enable();
//	//����MPU6500�жϻ���MCU
//	power_open_acel_int();
//	rt_pin_irq_enable(ACCEL_INT_PIN, PIN_IRQ_ENABLE);
//	//ʧ�� SysTick ������
//	//������ﲻʧ�� SysTick ����������ô�᲻�ܽ���͹���ģʽ��
//	SysTick->CTRL = 0x00;//�رն�ʱ��
//	SysTick->VAL = 0x00;//���val,��ն�ʱ��
	//����ģʽ
	rcu_periph_clock_enable(RCU_PMU);
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,GPIO_PIN_8);
	nvic_irq_enable(EXTI5_9_IRQn, 2U, 0U);
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_8);
	exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
	exti_interrupt_flag_clear(EXTI_8);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//ֹͣģʽ���Ѻ���Ҫ��������ϵͳʱ��
	SystemInit();

//	/* Enable SysTick IRQ and SysTick Timer */
//	SysTick->CTRL  = 	SysTick_CTRL_CLKSOURCE_Msk |
//						SysTick_CTRL_TICKINT_Msk   |
//						SysTick_CTRL_ENABLE_Msk;
}

/*!
 * \brief MCU����ģʽ
 *		  
 * \param  NONE
 *
 * \return NONE 
 */
void Mcu_PowerDown()
{
	int remainMin = 0;//Sign_GetRemainMin();
//	
//	g_isRtcTimeOut = False;
//	LOG2(ET_SYS_SLEEP, (uint8)remainMin, 0);//Mcu_GetRound5V());
	Printf("power down.\n");
	g_isPowerDown = True;
	Enter_PowerDown();
	g_isPowerDown = False;
//	LOG2(ET_SYS_WAKEUP, g_Settings.devcfg, GetWakeUpType());
//	BootWithReason(MCURST_PWR);
#ifdef CFG_SIM_SLEEP
	//SIM RX wake up
//	SIM_UART_RX_PORT->ISRC = SIM_UART_RX_PIN;
//    GPIO_EnableInt(SIM_UART_RX_PORT, SIM_UART_RX_PIN_IND, GPIO_INT_BOTH_EDGE); 
//#endif
//#ifdef CFG_BLE_SLEEP
//	//BLE RX wake up
// 	BLE_UART_RX_PORT->ISRC = BLE_UART_RX_PIN;
//   GPIO_EnableInt(BLE_UART_RX_PORT, BLE_UART_RX_PIN_IND, GPIO_INT_BOTH_EDGE); 
#endif

//	PMS_WAKEUP_SMART_PORT->ISRC = PMS_WAKEUP_SMART_PIN;
//	GPIO_EnableInt(PMS_WAKEUP_SMART_PORT, PMS_WAKEUP_SMART_PIN_IND, GPIO_INT_BOTH_EDGE);	
//	
//	g_isPowerDown = True;
//	CLK_PowerDown();
//	g_isPowerDown = False;
//	Sim_SetTxDataTime(GPRS_SENDDATA_TIME_SHORT);
//	
 	Printf("\nPower up.\n");	
//	LOG2(ET_SYS_WAKEUP, g_Settings.devcfg, g_WakeupType);
//	env_nvds_init();
//	Fsm_Init();
//	Fsm_Start();
	
}
/*!
 * \brief MCU����
 *		  
 * \param  NONE
 *
 * \return NONE 
 */
void Mcu_Sleep()
{
//	PFL(DL_MAIN, "Mcu sleep.\n");
	Mcu_PowerDown();
}

/*!
 * \brief main function
 *		  
 * \param[in]  none
 * \param[out] none
 *
 * \return     none
 */
int main(void)
{
	Printf("\nPower up.\n");	
	//MCUӲ����ʼ��
	//Todo...

	//���ж����ʼ��
	Led_init();
//	JT808_init();
	Pms_init();
	
	//��������
	ObjList_Start();

    while(1)
	{
		//��������
		ObjList_Run();
		rt_thread_mdelay(100);
    }
}

