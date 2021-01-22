
#include "drv_pm.h"
#include "Common.h"
#include "drv_rtc.h"
#include "drv_gpio.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//复位之前保存
	NvdsUser_Write(NVDS_CFG_INFO);
	NvdsUser_Write(NVDS_PDO_INFO);
	NvdsUser_Write(NVDS_DBG_INFO);
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //关闭所有中断
	NVIC_SystemReset();
}
volatile bool g_isPowerDown = False;//休眠标志，False-没有休眠，True-休眠

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

//处理延时复位功能，主要用于响应一些外部命令，执行后需要应答，然后执行MCU复位操作
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
 * \brief 设置休眠时IO电平，配置唤醒中断
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
	gJT808ExtStatus = _JT808_EXT_SLEEP ;	// 唤醒外置模块
#endif 

	gd32_hw_usart_deinit();

	PortPin_Set(g_pLedIO->periph, g_pLedIO->pin, True);

	RTC_TimerStart(12*60*60);			//定时12小时唤醒中控
	
	//待机模式
	rcu_periph_clock_enable(RCU_PMU);
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//停止模式唤醒后，需要重新配置系统时钟
	SystemInit();
	rt_thread_mdelay(200);
	//
	gd32_hw_usart_init();
	workmode_switchStatus(WM_ACTIVE);
	WorkMode_run();	// 跑一次,,, 放在这里不好看,或者把workmode 当成第一个任务
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
//	int remainMin = 0;//Sign_GetRemainMin();

	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SysSleep, 0);
	Printf("power down.\n");
	g_isPowerDown = True;
	// 清除所有唤醒状态
	g_WakeupType = 0x0000 ;
	
	Enter_PowerDown();
	
	g_isPowerDown = False;
 	Printf("\nPower up.\n");
	PortPin_Set(g_p18650PwrOffIO->periph, g_p18650PwrOffIO->pin, True);	
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_WakeUp, GetWakeUpType());
	Printf("wake up reason:%X\n",GetWakeUpType());
	
#ifdef _GENERAL_CENTRAL_CTL		// 普通中控
	{
		Pms_switchStatus(PMS_ACC_ON);
#if defined ( CANBUS_MODE_JT808_ENABLE ) && defined ( _GENERAL_CENTRAL_CTL )
		gJT808ExtStatus = _JT808_EXT_WAKUP ;	// 唤醒外置模块
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

