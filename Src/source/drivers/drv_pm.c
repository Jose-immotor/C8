
#include "drv_pm.h"
#include "Common.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//复位之前保存
	NvdsUser_Write(NVDS_PDO_INFO);
	NvdsUser_Write(NVDS_DBG_INFO);
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //关闭所有中断
	NVIC_SystemReset();
}
volatile bool g_isPowerDown = False;//休眠标志，False-没有休眠，True-休眠

//WakeupType g_WakeupType;
//WakeupType GetWakeUpType()
//{
//	return g_WakeupType;
//}
//void SetWakeUpType(WakeupType type)
//{
//	g_WakeupType = type;
//}

//处理延时复位功能，主要用于响应一些外部命令，执行后需要应答，然后执行MCU复位操作
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

/*!
 * \brief 设置休眠时IO电平，配置唤醒中断
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
void Enter_PowerDown()
{	
//	RTC_TimerStart(24*60*60);
//	//485 power
//	POWER_3V3_485_OFF;
//	//NFC power
//	rt_pin_write(FM17522_ON, PIN_LOW);
//	rt_pin_write(FM17522_ON1, PIN_LOW);
//	ADC_Ctrl(ADC1, DISABLE);
//	//失能 SysTick 计数器
//	//如果这里不失能 SysTick 计数器，那么会不能进入低功耗模式的
//	SysTick->CTRL = 0x00;//关闭定时器
//	SysTick->VAL = 0x00;//清空val,清空定时器
	//待机模式
	rcu_periph_clock_enable(RCU_PMU);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//停止模式唤醒后，需要重新配置系统时钟
	SystemInit();
	rt_thread_mdelay(200);
	Pms_switchStatus(PMS_ACC_OFF);
//	/* Enable SysTick IRQ and SysTick Timer */
//	SysTick->CTRL  = 	SysTick_CTRL_CLKSOURCE_Msk |
//						SysTick_CTRL_TICKINT_Msk   |
//						SysTick_CTRL_ENABLE_Msk;
}

/*!
 * \brief MCU待机模式
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
 * \brief MCU休眠
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

