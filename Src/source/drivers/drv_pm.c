
#include "drv_pm.h"
#include "Common.h"
#include "drv_rtc.h"
#include "drv_gpio.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//��λ֮ǰ����
	NvdsUser_Write(NVDS_CFG_INFO);
	NvdsUser_Write(NVDS_PDO_INFO);
	NvdsUser_Write(NVDS_DBG_INFO);
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //�ر������ж�
	NVIC_SystemReset();
}
volatile bool g_isPowerDown = False;//���߱�־��False-û�����ߣ�True-����

static WakeupType g_WakeupType;
WakeupType GetWakeUpType(void)
{
	return g_WakeupType;
}
void SetWakeUpType(WakeupType type)
{
	g_WakeupType |= type;
}
void ClearWakeupType(void)
{
	g_WakeupType = 0x00;
}

//������ʱ��λ���ܣ���Ҫ������ӦһЩ�ⲿ���ִ�к���ҪӦ��Ȼ��ִ��MCU��λ����
//uint32 g_ResetMs = 0;
//static uint32_t g_ResetInitMs = 0;
//static Bool g_isPowerOff = False;
//void ResetDelay(MCURST reason, uint32 ms);
void DelayPowerOff(uint32 ms)
{
//	ResetDelay(MCURST_18650_POWER_OFF, ms);
//	g_isPowerOff = True;
}
void ResetStop()
{
//	g_ResetMs = 0;
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
extern DrvIo* g_p18650PwrOffIO;
extern void WorkMode_run();
#ifdef CANBUS_MODE_JT808_ENABLE
extern JT808ExtStatus gJT808ExtStatus ;
extern void JT808CAN_Sleep(void);
extern DrvIo* g_pNfcNpdBIO ;
#endif 


void Enter_PowerDown()
{	
	extern DrvIo* g_pLedIO;
	//DrvIo* g_pNfcON = Null;
	
	if((g_cfgInfo.isActive == 0)&&(
		g_Bat[0].presentStatus != BAT_IN || g_Bat[1].presentStatus != BAT_IN
		))
		PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, False);
	else
		PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, True);	
	
#ifdef CANBUS_MODE_JT808_ENABLE
	JT808CAN_Sleep();
	gJT808ExtStatus = _JT808_EXT_SLEEP ;	// ��������ģ��
#endif 

	gd32_hw_usart_deinit();

	PortPin_Set(g_pLedIO->periph, g_pLedIO->pin, True);

	RTC_TimerStart(12*60*60);			//��ʱ12Сʱ�����п�
	
	//����ģʽ
	rcu_periph_clock_enable(RCU_PMU);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//ֹͣģʽ���Ѻ���Ҫ��������ϵͳʱ��
	SystemInit();
	rt_thread_mdelay(200);
	//
	gd32_hw_usart_init();
	workmode_switchStatus(WM_ACTIVE);
	WorkMode_run();	// ��һ��,,, �������ﲻ�ÿ�,���߰�workmode ���ɵ�һ������
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
//	int remainMin = 0;//Sign_GetRemainMin();

	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SysSleep, 0);
	Printf("power down.\n");
	g_isPowerDown = True;
	// ������л���״̬
	g_WakeupType = 0x0000 ;
	
	Enter_PowerDown();
	
	g_isPowerDown = False;
 	Printf("\nPower up.\n");
	PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, True);	
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_WakeUp, GetWakeUpType());
	Printf("wake up reason:%X\n",GetWakeUpType());
	
#ifdef _GENERAL_CENTRAL_CTL		// ��ͨ�п�
	{
		Pms_switchStatus(PMS_ACC_ON);
#if defined ( CANBUS_MODE_JT808_ENABLE ) && defined ( _GENERAL_CENTRAL_CTL )
		gJT808ExtStatus = _JT808_EXT_WAKUP ;	// ��������ģ��
#endif //		
	}
#else
	if(g_cfgInfo.isAccOn)
	{
		Pms_switchStatus(PMS_ACC_ON);
	}
	else
	{
		Pms_switchStatus(PMS_ACC_OFF);
	}
#endif 	
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

