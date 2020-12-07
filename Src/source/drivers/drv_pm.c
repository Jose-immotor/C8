
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

WakeupType g_WakeupType;
WakeupType GetWakeUpType()
{
	return g_WakeupType;
}
void SetWakeUpType(WakeupType type)
{
	g_WakeupType = type;
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
void Enter_PowerDown()
{	
	extern DrvIo* g_pLedIO;
	//DrvIo* g_pNfcON = Null;
	
	if((g_cfgInfo.isActive == 0)&&(g_Bat[0].presentStatus != BAT_IN))
		PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, False);
	else
		PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, True);
	

	PortPin_Set(g_pLedIO->periph, g_pLedIO->pin, True);

	// ������ڳ��ʱ����,��5��������һ�Σ����Ƿ����
	if( g_pdoInfo.isLowPow && 
		( g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN ) )	// �ǵ͵�ʱ,ֱ�ӹر�BAT
	{
		RTC_TimerStart(5*60);//��ʱ5���ӻ����п�
	}
	else
	{
		RTC_TimerStart(6*60*60);//��ʱ6Сʱ�����п�
	}
	//����ģʽ
	rcu_periph_clock_enable(RCU_PMU);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//ֹͣģʽ���Ѻ���Ҫ��������ϵͳʱ��
	SystemInit();
	rt_thread_mdelay(200);
	//
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
	g_WakeupType = WAKEUP_MAX ;
	
	Enter_PowerDown();
	
	g_isPowerDown = False;
 	Printf("\nPower up.\n");
	PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, True);	
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_WakeUp, GetWakeUpType());
	Printf("wake up reason:%d\n",GetWakeUpType());
#ifdef SPECIAL_EDITION_AUTOACCON		// ����汾
	if( g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN )
	{
		Pms_switchStatus(PMS_ACC_ON);
	}
	else
	{
		Pms_switchStatus(PMS_ACC_OFF);
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

