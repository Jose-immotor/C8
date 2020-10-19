
#ifndef __WORKMODE_H__
#define __WORKMODE_H__

#include "Bit.h"
#include "Message.h"

#define WORKMODE_FORCE_SLEEP_TIME 5*1000//5秒

	//活动标志定义，当该标志被置位时，设备不能进入睡眠。
	typedef enum _ActiveFlag
	{
		 AF_MDB = BIT_0			//modbus处于工作状态
		,AF_NVC = BIT_1			//NVC正在播放语音
		,AF_CABIN = BIT_2		//座舱锁处于工作状态
		,AF_PMS = BIT_3			//PMS处于工作状态
		,AF_NFC = BIT_4			//NFC处于工作状态
	//	
	//	,AF_PMS = BIT_16			//PMS
	//	,AF_BEACON = BIT_17			//BEACON是否连接状态
	//	
	//	,AF_FW_UPGRADE = BIT_24	//正在启动升级操作
	}ActiveFlag;

	//work状态
	typedef enum _WorkmodeOpStatus
	{
		WM_ACTIVE = 1,	//工作
		WM_SLEEP	,	//休眠
	}WorkmodeOpStatus;
	
	typedef void (*WorkMode_FsmFn)(uint8_t msgId, uint32_t param1, uint32_t param2);
	typedef struct _workmode
	{
		//workmode操作状态
		WorkmodeOpStatus opStatus;
		//状态机函数指针，由status决定指向那个状态处理函数
		WorkMode_FsmFn Fsm;
		
		uint32_t statusSwitchTicks;
		
		uint8_t first_flag;
		uint8_t active_flag;
		uint8_t sleep_flag;

	}workmode;


void WorkMode_init();
void workmode_switchStatus(WorkmodeOpStatus newStatus);
void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);	
#endif
