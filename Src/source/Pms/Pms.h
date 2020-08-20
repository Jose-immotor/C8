
#ifndef __BATTERY_MGR_H_
#define __BATTERY_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "ModBus.h"
#include "BmsReg.h"
#include "queue.h"
#include "Message.h"

	//电池操作状态
	typedef enum _PmsOpStatus
	{
		PMS_ACC_OFF	,		//熄火,初始状态
		PMS_ACC_ON	,		//点火
		PMS_SLEEP	,		//浅休眠
		PMS_DEEP_SLEEP,	//深度休眠
	}PmsOpStatus;

	//PMS消息定义
	typedef enum _PmsMsg
	{
		PmsMsg_run = 1,		//运行，消息格式：(BmsMsg_run, 0, 0)
		PmsMsg_timeout,		//超时，消息格式：(BmsMsg_timeout, timerID, 0)
		
		PmsMsg_batPlugIn,	//电池插入，消息格式：(BmsMsg_batPlugIn, 0, 0)
		PmsMsg_batPlugout,	//电池拔出，消息格式：(BmsMsg_batPlugout(0, 0)

		PmsMsg_accOn,	//电池插入，消息格式：(BmsMsg_batPlugIn, 0, 0)
		PmsMsg_accOff,	//电池拔出，消息格式：(BmsMsg_batPlugout(0, 0)

		PmsMsg_sleep,		//电池浅休眠，消息格式：(BmsMsg_sleep, 0, 0)
		PmsMsg_deepSleep,	//电池深休眠，消息格式：(BmsMsg_deepSleep, 0, 0)
		PmsMsg_wakeup,		//电池唤醒，消息格式：(BmsMsg_wakeup, 0, 0)
	}PmsMsg;

	typedef struct _Pms
	{
		Mod modBus;

		//Pms操作状态
		PmsOpStatus opStatus;

		//消息定义
		Message msgBuf[32];
		Queue msgQueue;
	}Pms;

	extern Mod* g_pModBus;

	//从总线接收到的数据，调用该函数处理
	void Pms_init();
	void Pms_Rx(int nfcPort, const uint8_t* pData, int len);
	void Pms_postMsg(uint8_t msgId, uint32_t param1, uint32_t param2);

#ifdef __cplusplus
}
#endif

#endif


