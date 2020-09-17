
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
#define PMS_ACC_OFF_ACTIVE_TIME 5*60*1000//5分钟

	//活动标志定义，当该标志被置位时，设备不能进入睡眠。
	typedef enum _ActiveFlag
	{
		 AF_MDB = BIT_0			//modbus处于工作状态
		,AF_NVC = BIT_1			//NVC正在播放语音
	//	,AF_MCU_RESET = BIT_1	//MCU处于延时复位状态
		
	//	,AF_SIM = BIT_8			//SIM模组有命令没有发完
	//	,AF_CARBIN_LOCK = BIT_9	//正在操作座舱锁
	//	
	//	,AF_PMS = BIT_16			//PMS
	//	,AF_BEACON = BIT_17			//BEACON是否连接状态
	//	
	//	,AF_FW_UPGRADE = BIT_24	//正在启动升级操作
	}ActiveFlag;

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
	}PmsMsg;

	struct _Pms;
	typedef void (*Pms_FsmFn)(PmsMsg msgId, uint32_t param1, uint32_t param2);
	typedef struct _Pms
	{
		Mod modBus;

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

	NfcCardReaderEventRc Pms_cardReaderEventCb(Battery* pBat, NfcCardReaderEvent ev);
	void Pms_switchStatus(PmsOpStatus newStatus);
	void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);
#ifdef __cplusplus
}
#endif

#endif


