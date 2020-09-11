
#include "drv_pm.h"
#include "Common.h"
#include "drv_rtc.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//��λ֮ǰ����
	NvdsUser_Write(NVDS_PDO_INFO);
	NvdsUser_Write(NVDS_DBG_INFO);
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //�ر������ж�
	NVIC_SystemReset();
}
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

/*!
 * \brief ��������ʱIO��ƽ�����û����ж�
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
//	//ʧ�� SysTick ������
//	//������ﲻʧ�� SysTick ����������ô�᲻�ܽ���͹���ģʽ��
//	SysTick->CTRL = 0x00;//�رն�ʱ��
//	SysTick->VAL = 0x00;//���val,��ն�ʱ��
	//����ģʽ
	rcu_periph_clock_enable(RCU_PMU);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//ֹͣģʽ���Ѻ���Ҫ��������ϵͳʱ��
	SystemInit();
	rt_thread_mdelay(200);
	Pms_switchStatus(PMS_ACC_OFF);
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

