
#ifndef __PMS_H_
#define __PMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "ModBus.h"
#include "BmsReg.h"
#include "queue.h"
#include "Message.h"
#include "Battery.h"
#include "Bit.h"

#define MAX_BAT_COUNT 2
#define PMS_ACC_OFF_ACTIVE_TIME 	(30*1000)//30秒
#define PMS_ACC_DEEPSLEEP_TIME 		(10*1000)//(5*60*1000)//5分钟

//
#define	PMS_LOW_CURRENT_2A				(200)	// 实际为 2A

#define	PMS_ACC_OFF_TIME				(5*60*1000)		// 放电电流小于2A持续 5分钟，则关闭外置模块
#define	PMS_ACC_OFF_MODE_WAKUP_TIME		(12*60*60*1000) //(10*60*1000)//	// 12小时后，唤醒外置模块
#define	PMS_ACC_DEPSLEEP_TIME			(48*60*60*1000)	// (15*60*1000)// // 放电电流小于2A持续 2天,关闭电池,休眠之



typedef enum
{
	_JT808_EXT_SLEEP = 0,
	_JT808_EXT_BRIEF_WAKUP,
	_JT808_EXT_WAKUP
}JT808ExtStatus;


	//电池操作状态
	typedef enum _PmsOpStatus
	{
		PMS_ACC_OFF = 1,	//熄火,初始状态
		PMS_ACC_ON	,		//点火
		PMS_SLEEP	,		//浅休眠
		PMS_DEEP_SLEEP,		//深度休眠
		PMS_CAN_ERR,		//CAN通信错误
	}PmsOpStatus;

	//PMS消息定义
	typedef enum _PmsMsg
	{
		PmsMsg_run = 1,		//运行，消息格式：(BmsMsg_run, 0, 0)
		PmsMsg_timeout,		//超时，消息格式：(BmsMsg_timeout, timerID, 0)
		
		PmsMsg_batPlugIn,	//电池插入，消息格式：(BmsMsg_batPlugIn, port, 0)
		PmsMsg_batPlugOut,	//电池拔出，消息格式：(BmsMsg_batPlugout, port, 0)

		PmsMsg_accOn,	//电池插入，消息格式：(BmsMsg_batPlugIn, 0, 0)
		PmsMsg_accOff,	//电池拔出，消息格式：(BmsMsg_batPlugout0, 0)

		PmsMsg_sleep,		//电池浅休眠，消息格式：(BmsMsg_sleep, 0, 0)
		PmsMsg_deepSleep,	//电池深休眠，消息格式：(BmsMsg_deepSleep, 0, 0)
		PmsMsg_wakeup,		//电池唤醒，消息格式：(BmsMsg_wakeup, 0, 0)
		
		PmsMsg_GyroIrq,		//陀螺仪震动，消息格式：(PmsMsg_GyroIrq, 0, 0)
#ifdef CANBUS_MODE_JT808_ENABLE	
		PmsMsg_GPRSIrq,		// GPRS中断
#endif
		PmsMsg_18650Low,	// 18650 低电
		PmsMsg_18650Normal, // 18650 电量正常
	}PmsMsg;

	struct _Pms;
	typedef void (*Pms_FsmFn)(PmsMsg msgId, uint32_t param1, uint32_t param2);
	typedef struct _Pms
	{
		Mod modBus;
		Fm175Drv fmDrv;	//NFC驱动对象

		//Pms操作状态
		PmsOpStatus opStatus;
		//状态机函数指针，由status决定指向那个状态处理函数
		Pms_FsmFn Fsm;
		uint32_t statusSwitchTicks;

		//消息定义
		Message msgBuf[32];
		Queue msgQueue;
	}Pms;

	extern Mod* g_pModBus;
	extern Battery g_Bat[];

	//从总线接收到的数据，调用该函数处理
	void Pms_init();
	void Pms_Rx(Battery* pBat, const uint8_t* pData, int len);
	void Pms_postMsg(PmsMsg msgId, uint32_t param1, uint32_t param2);

	TRANSFER_EVENT_RC Pms_EventCb(Battery* pBat, TRANS_EVENT ev);
	void Pms_switchStatus(PmsOpStatus newStatus);
	PmsOpStatus Pms_GetStatus(void);
#ifdef __cplusplus
}
#endif

#endif


