/*******************************************************************************
  * Copyright (C) 2020 by IMMOTOR
  *
  * File   : fsm.c
  * 
  * 状态机
  *
  * Change Logs:
  * Date		   Author		 Notes
  * 2020-05-25		lane	 first implementation
*******************************************************************************/
#include "common.h"
#include "fsm.h"
#include "rtthread.h"
#include "pms.h"
#include "nvc.h"
#include "smart_system.h"
#include "main.h"
#include "msgdef.h"
#include "nvds.h"

//static Queue g_Queue;//g_Queue_fsm的前两个地址被未知修改了，所以增加定义这个变量
static Queue g_Queue_fsm;
static Message* g_pMsg = Null;
static FSM_STATE g_FsmState;
static uint32 g_isGpsUpdated = 0;
static SwTimer g_FsmTimer;
uint32 g_ActiveFlag = 0;
#define MAX_GPS_COUNT 5
#define MIN_GPS_COUNT 1
int g_MaxGpsCount = MIN_GPS_COUNT;

#define FSM_TIMERID_POWERDOWN 			0
//#define TIME_WAITFOR_END 			(2000)	//等待网络传输结束

#define FSM_PFL PFL
const char* g_pStateStr[] = 
{
	 "FSM_KEY_OFF"
	,"FSM_KEY_ON"
	,"FSM_POWERDOWN"
	,"FSM_FW_UPGRADE"
	,"FSM_KEY_CNT"
	,"FSM_MAX"
};

const char* g_pMsgStr[] = 
{
	 "TIMEOUT"
	,"RUN"
	,"UTP_REQ_DONE"
	,"KEY_ON"
	,"KEY_OFF"
	,"BATTERY_PLUG_IN"
	,"BATTERY_PLUG_OUT"
	,"SIM_POWEROFF"
	,"12V_OFF"
	,"12V_ON"
	
	,"SIM_FAILED"
	,"GPS_UPDATE"
	,"GPRS_UPDATE "
	,"BLE_CNT"
	,"BLE_DIS_CNT"
	,"PMS_WAKEUP"
	,"PMS_SLEEP"
	,"5V_OFF"
	,"5V_ON"
	,"BATTERY_PLUG_IN"
	,"BATTERY_PLUG_OUT"
	,"CHARGER_PLUG_IN"
	,"CHARGER_PLUG_OUT"
	,"RTC_TIMEOUT"
	,"FW_UPGRADE"
	,"SIM_WAKEUP"
	,"LOCK_KEY"
	,"GPS_POWERON"
	,"GPS_POWEROFF"
	,"SIM_POWERON"
	
 	,"FW_UPGRADE_DONE"	//= 28
	,"SIM_COMM "
	,"SIM_CARD_READY "
	,"BLE_WAKEUP "
	,"BATTERY_FAULT "
	,"MSG_GPS_MOVE"
	,"MSG_GPS_STOP"
	,"MAX"
};

void Fsm_StateKeyOff(uint8 msgID);

void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive)
{
	if(isActive)
	{
		g_ActiveFlag |= af;
	}
	else
	{
		g_ActiveFlag &= ~af;
	}
}

//Bool Fsm_CanGoSleep(){ return g_ActiveFlag == 0;}
////uint8 Fsm_Get(void)
////{
////	return g_FsmState;
////}

void PostMsg(uint8 msgId)
{
	Message msg = {0};
	
	msg.m_MsgID  = msgId;
	msg.m_Param1 = 0;
	Queue_write(&g_Queue_fsm, &msg, sizeof(Message));
}

///*!
// * \brief 设置状态机
// *		  
// * \param  
// *
// * \return NONE 
// */
//static void Po(int argc, char**argv)
//{
//	int msgId;
//	
//	sscanf(&(*argv[1]), "%d", &msgId);
//	PostMsg(msgId);
//}
//MSH_CMD_EXPORT(Po, sample: Po <uint8 msgId>);

static void Fsm_TimerStart(uint32 timeOutMs, uint32 nId)
{
	FSM_PFL(DL_FSM, "Timer[%d] start(%ds).\n", nId, timeOutMs/1000);
	SwTimer_Start(&g_FsmTimer, timeOutMs, nId);
}

//void Fsm_SetMaxGpsCount(int value)
//{
//	g_MaxGpsCount = value;
//	g_isGpsUpdated = 0;
//	Sim_Wakeup();
//	Fsm_TimerStart(TIME_UPDATE_GPS_TO_SERVER, FSM_TIMERID_POWERDOWN);
//}

//void Fsm_GoAlarmMode(Bool isEnable)
//{
//	g_Settings.IsAlarmMode = isEnable;
//	
//	g_MaxGpsCount = IsAlarmMode() ? MAX_GPS_COUNT : MIN_GPS_COUNT;
//	LOG2(IsAlarmMode() ? ET_SYS_ALARM_MODE_ON : ET_SYS_ALARM_MODE_OFF, g_Settings.devcfg, 0);
//	if(IsAlarmMode())
//	{
//		Beep_Mode(BEEP_ALARM_MODE_EN);
//	}
//	else
//	{
//		Beep_Mode(BEEP_ALARM_MODE_DIS);
//	}

//	Gprs_SetHearbeatInterval(10);
//}

int g_GyroIsrCounter = 0;
void Fsm_SetState(FSM_STATE state)
{
	extern void Mcu_Sleep();
	if(g_FsmState == state) return;

	FSM_PFL(DL_FSM, "State[%s] to [%s]\n", g_pStateStr[g_FsmState], g_pStateStr[state]);
	SwTimer_Stop(&g_FsmTimer);
	g_GyroIsrCounter = 0;
	g_isGpsUpdated = 0;
	g_MaxGpsCount = IsAlarmMode() ? MAX_GPS_COUNT : MIN_GPS_COUNT;
	if(FSM_KEY_OFF == state)
	{
		if(FSM_POWERDOWN == g_FsmState)
		{
		#ifdef CFG_SIM_SLEEP
//			Sim_Wakeup();
		#else
//			Sim_PowerOn();
		#endif
			Fsm_TimerStart(TIME_UPDATE_GPS_TO_SERVER, FSM_TIMERID_POWERDOWN);
		}
		//设置GPS连接超时时间5分钟，在KeyOff状态，5分钟后没有定位成功，则强制休眠	
//		Fsm_StateKeyOff(MSG_RUN);
	}
	else if(FSM_POWERDOWN == state)
	{	
	#ifdef CFG_SIM_SLEEP
//		Sim_Sleep();		
		Fsm_TimerStart(1000, 0);
	#else
//		Sim_PowerOff();
	#endif
//#ifdef CFG_BLE_SLEEP
//		if(!IS_BAT_ON())
//#endif	
//		{
//			Ble_Sleep();
//		}
//		if(!IsActive() && g_pPms->m_portMask == 0)
//			DelayPowerOff(10);
//		else
//			Mcu_Sleep();
	}
	else if(FSM_KEY_ON == state)
	{		
//		Mcu_Wakeup();
//		Ble_Wakeup();
//		Sim_PowerOn();
	}
	else if(FSM_FW_UPGRADE == state)
	{
//		Sim_PowerOff();
	}
	
	g_FsmState = state;
}
//ACC OFF时处理逻辑
void Fsm_StateKeyOff(uint8 msgID)
{
	if(MSG_TIMEOUT == msgID)// || MSG_SIM_FAILED == msgID)
	{
		//检查是否可以进入睡眠
		if(g_ActiveFlag)
		{
			Fsm_TimerStart(1000, FSM_TIMERID_POWERDOWN);
		}
	#ifdef CFG_SIM_SLEEP
//		else if(!Sim_IsSleep())
//		{
//			Sim_Sleep();
//		}
	#endif
		else
		{
			PostMsg(MSG_SIM_POWEROFF);
		}
	}
	else if(MSG_SIM_POWEROFF == msgID)
	{
		if(g_ActiveFlag)
		{
			Fsm_TimerStart(1000, FSM_TIMERID_POWERDOWN);
		}
		else
		{
			Fsm_SetState(FSM_POWERDOWN);
		}
	}
//	else if(MSG_BLE_CNT == msgID)
//	{
//		SwTimer_Stop(&g_FsmTimer);
//	}
//	else if(MSG_BLE_DIS_CNT == msgID)
//	{
//		Fsm_TimerStart(TIME_UPDATE_GPS_TO_SERVER, FSM_TIMERID_POWERDOWN);

////		Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
//	}
//	else if(MSG_FORCE_POWERDOWN == msgID)//强制休眠
//	{
//		Fsm_TimerStart(1000, FSM_TIMERID_POWERDOWN);

////		Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
//	}
	else if(MSG_RUN == msgID)//5分钟定位不成功，休眠
	{
		Fsm_TimerStart(TIME_UPDATE_GPS_TO_SERVER, FSM_TIMERID_POWERDOWN);
//		Fsm_TimerStart(10000, FSM_TIMERID_POWERDOWN);
	}
//	else if(MSG_GPS_UPDATE == msgID)
//	{
//		g_isGpsUpdated++;
//		if(g_isGpsUpdated < g_MaxGpsCount)
//		{
//			SwTimer_ReStart(&g_FsmTimer);
//		}
//		else
//		{
////			Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
//		}
//		SwTimer_Stop(&g_FsmTimer);
//	}
//	else if(MSG_GPRS_UPDATE == msgID)
//	{
//		//PFL(DL_GPS, "MSG_GPRS_UPDATE: %d,%d,%d\n", g_isGpsUpdated, g_MaxGpsCount, g_pBle->isConnected);
//		//Gprs_SetHearbeatInterval(g_isGpsUpdated ? TIME_HEARBEAT_LONG : TIME_HEARBEAT_LONG_LONG);
////		Gprs_SetHearbeatInterval(TIME_HEARBEAT_LONG_LONG);
////		if(g_isGpsUpdated >= g_MaxGpsCount && !g_pBle->isConnected)// && 0)
////		{
////			#ifdef CFG_SIM_SLEEP
////			Sim_Sleep();
////			#else
////			//不能立刻切换到关机状态，必须延时2S等待GPRS模组传输数据结束
////			Fsm_TimerStart(TIME_WAITFOR_END, FSM_TIMERID_POWERDOWN);
////			#endif
////		}
//		
//	}
	else if(MSG_KEY_ON == msgID)
	{
		Fsm_SetState(FSM_KEY_ON);
	}
	else if(MSG_BATTERY_PLUG_IN == msgID)// && MSG_12V_ON == msgID )
	{
//		if(Pms_IsAccOn())
//		{
//			Fsm_SetState(FSM_KEY_ON);
//		}
	}
//	else if(MSG_GYRO_ASSERT == msgID)
//	{
//#ifdef CFG_NVC	
//		if(IsAlarmMode()) 
//		{
//			NVC_PLAY(NVC_WARNING);
//		}
//#endif	
//		Fsm_TimerStart(TIME_UPDATE_GPS_TO_SERVER, FSM_TIMERID_POWERDOWN);
//	}
	
}

void Fsm_StateKeyOn(uint8 msgID)
{
	if(MSG_KEY_OFF == msgID || MSG_12V_OFF == msgID)
	{
//		Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
		Fsm_SetState(FSM_KEY_OFF);
#ifdef CFG_NVC	
		Nvc_Reset();
#endif		
	}
//	else if(MSG_GPS_UPDATE == msgID)
//	{
//		g_isGpsUpdated++;
//		//Gprs_SetHearbeatInterval(TIME_HEARBEAT_SHORT);
//	}
//	else if(MSG_GPRS_UPDATE == msgID)
//	{
//		//Gprs_SetHearbeatInterval(g_isGpsUpdated ? TIME_HEARBEAT_LONG : TIME_HEARBEAT_LONG_LONG);
////		Gprs_SetHearbeatInterval(TIME_HEARBEAT_LONG_LONG);
//	}
//	else if(MSG_GYRO_ASSERT == msgID)
//	{
//#ifdef CFG_NVC	
//		if(IsAlarmMode()) 
//		{
//			NVC_PLAY(NVC_WARNING);
//		}
//#endif		
//	}
}

void Fsm_StatePowerDown(uint8 msgID)
{
	extern void Mcu_PowerDown();
	/*
	if(MSG_RTC_TIMEOUT == msgID)
	{
		Printf("Slience = 1\n");
		g_pDataRom->m_isSlience = 1;
		DataRom_Write();
		BootWithReason(False);
	}
	*/
//	if(MSG_PMS_WAKEUP == msgID 
//		|| MSG_RTC_TIMEOUT == msgID
//		|| MSG_SIM_WAKEUP == msgID
//		|| MSG_LOCK_KEY == msgID
//		|| MSG_GPRS_UPDATE == msgID
//		|| MSG_ADC_ISR == msgID
//		|| MSG_BLE_WAKEUP == msgID
//#ifdef CFG_CABIN_LOCK	
//		|| MSG_CABIN_LOCK_CHANGED == msgID
//#endif	
//		)
//	{
//		Fsm_SetState(FSM_KEY_OFF);
//#ifdef CFG_SIM_SLEEP
////		Sim_SendCmdInd(CMD_SIM_CSCLK);
//#endif
//		if(MSG_BLE_WAKEUP == msgID)
//		{
////			Ble_DoConnected(Null);
//		}
//	}
//	else if(MSG_GYRO_IDLE == msgID || MSG_SIM_FAILED == msgID || MSG_TIMEOUT== msgID)
	if(MSG_TIMEOUT== msgID)
	{
		g_GyroIsrCounter = 0;
		//Sim_Reset();
		Fsm_TimerStart(1000, 0);
		Mcu_PowerDown();
	}
//	else if(MSG_BLE_WAKEUP == msgID)
//	{
//		Fsm_TimerStart(1000, 0);
//	}
//	else if(MSG_GYRO_ASSERT == msgID)
//	{
//		#if 0
//		//在30秒内连续触发3次(每10秒/次)，认为是异动唤醒。
//		Fsm_TimerStart(10000, 0);
//		
////		Sim_Reset();
//		if(++g_GyroIsrCounter >= 3)
//		{
//			Beep_Mode(BEEP_GRYO);
//			Fsm_StatePowerDown(MSG_LOCK_KEY);
//		}
//		#else
//#ifdef CFG_RENT
//		//没有语音提示.
//		Fsm_StateKeyOff(MSG_GYRO_ASSERT);
//		//Fsm_StatePowerDown(MSG_LOCK_KEY);
//#else
//	#ifdef CFG_NVC
//		NVC_PLAY(IsAlarmMode() ? NVC_INFO : NVC_WARNING);
//	#else
////		Beep_Mode(BEEP_GRYO);
//	#endif
//		Fsm_StatePowerDown(MSG_LOCK_KEY);
//#endif
//		#endif
//	}
}

void Fsm_StateFwUpgrade(uint8 msgID)
{
//	if(msgID == MSG_FW_UPGRADE_DONE)
//	{
//		//如果是通过蓝牙升级，需要开启升级提示音，如果通过网络升级，保存静默
//		if(g_Settings.isBleUpg)
//		{
//			Beep_Mode(BEEP_UPGRADE_END);
//		}
//	}
}

void Fsm_MsgProc(void)
{
	int i = 0;
	struct
	{
		uint8 state;
		void (*Handler)(uint8 msgID);
	}
	static const map[] = 
	{
		 {FSM_KEY_OFF	, Fsm_StateKeyOff}
		,{FSM_KEY_ON	, Fsm_StateKeyOn}
		,{FSM_POWERDOWN	, Fsm_StatePowerDown}
		,{FSM_FW_UPGRADE, Fsm_StateFwUpgrade}
	};

	//消息处理
	for(g_pMsg = (Message*)Queue_Read(&g_Queue_fsm); g_pMsg;Queue_pop(&g_Queue_fsm), g_pMsg = (Message*)Queue_Read(&g_Queue_fsm))
	{
		//打印消息
		if(g_pMsg->m_MsgID < GET_ELEMENT_COUNT(g_pMsgStr))
		{
			if(g_FsmTimer.m_isStart)
			{
				FSM_PFL(DL_FSM, "%s: Proc %s(%d)\n", g_pStateStr[g_FsmState], g_pMsgStr[g_pMsg->m_MsgID], g_pMsg->m_MsgID);
			}
			else
			{
				FSM_PFL(DL_FSM, "%s: Proc %s(%d), TimeID=%d\n", g_pStateStr[g_FsmState], g_pMsgStr[g_pMsg->m_MsgID], g_pMsg->m_MsgID, g_FsmTimer.m_Id);
			}
		}
		else
		{
			FSM_PFL(DL_FSM, "%s: Proc (%d)\n", g_pStateStr[g_FsmState], g_pMsg->m_MsgID);
		}
		
		//处理电池和充电器插入
		if(MSG_12V_ON == g_pMsg->m_MsgID)
		{
			ResetStop();
		}
		//发现电池插入或者拔出，立刻触发网络上报
		if(MSG_BATTERY_PLUG_IN == g_pMsg->m_MsgID || MSG_BATTERY_PLUG_OUT == g_pMsg->m_MsgID)
		{
//			Gprs_SetHearbeatInterval(1);
			//所有电池拔出
//			if((g_pPms->m_portMask == 0)&& g_ResetMs == 0)
//			{
//				//如果是测试电池拔出引起的复位，则恢复出厂设置
//				if(g_pPms->m_isTestBat)
//				{
//					Nvds_Reset();
//					DelayPowerOff(1500);
//				}
////					else if(!g_pBle->isConnected)
////					{
////						if(!IsActive())
////							DelayPowerOff(1500);
////						else
////							ResetDelay(MCURST_ALL_BAT_PLUG_OUT, 5000);
////					}
//			}
		}
//		else if(MSG_BATTERY_FAULT == g_pMsg->m_MsgID)
//		{
//			//Beep_Mode(BEEP_BAT_FAULT);
//		}
//		else if(MSG_FW_UPGRADE == g_pMsg->m_MsgID)
//		{
//			Fsm_SetState(FSM_FW_UPGRADE);
//			continue;
//		}
		for(i = 0; i < GET_ELEMENT_COUNT(map); i++)
		{
			if(g_FsmState == map[i].state)
			{
				map[i].Handler(g_pMsg->m_MsgID);
				break;
			}
		}
	}
}

/*!
 * \brief 状态机运行
 *		  
 * \param 
 *
 * \return none
 */
void Fsm_Run(void)
{
	Fsm_MsgProc();	
	if(SwTimer_isTimerOut(&g_FsmTimer))
	{
		PostMsg(MSG_TIMEOUT);
	}
}

void Fsm_Start(void)
{
	if(g_Settings.isRemoteAccOn == True)
		Fsm_SetState(FSM_KEY_ON);
	else
		Fsm_SetState(FSM_KEY_OFF);
}

void Fsm_Init(void)
{
	static Message 	g_QueueBuff[10];
	Queue_init(&g_Queue_fsm, g_QueueBuff, sizeof(Message), sizeof(g_QueueBuff)/sizeof(Message));

	g_FsmState = FSM_MAX;
	
	g_MaxGpsCount = (IsAlarmMode()) ? MAX_GPS_COUNT : MIN_GPS_COUNT;
}
void thread_fsm_entry(void* parameter)
{
	Fsm_Init();	
	rt_thread_mdelay(1000);
	Fsm_Start();
    while (1)
    {
		Fsm_Run();
		rt_thread_mdelay(100);
    }
}

struct rt_thread thread_fsm;
unsigned char thread_fsm_stack[1024];

static int app_fsm_init(void)
{
	rt_err_t res;
	res=rt_thread_init(&thread_fsm,"fsm",thread_fsm_entry, RT_NULL,&thread_fsm_stack[0],
    sizeof(thread_fsm_stack), 1, 10);	
    if (res == RT_EOK) /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(&thread_fsm);
	else
		rt_kprintf("\n!!create thread fsm failed!\n");
    return 0;
}
INIT_APP_EXPORT(app_fsm_init);
