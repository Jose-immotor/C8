#ifndef __ARCH_CFG_H_
#define __ARCH_CFG_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
	
	#include <windows.h>
	#include <stdio.h>

	#define SLEEP 		Sleep
	#define GET_TICKS() system_ms_tick	//GetTickCount()
	#define UDELAY(...)
	
	extern CRITICAL_SECTION g_CriticalSection;
	#define Printf printf
#else
	#define MDELAY(mS) TIMER_Delay(TIMER0, (mS)*1000)
	#define UDELAY(uS) TIMER_Delay(TIMER0, (uS))
	
	#define GET_TICKS( ) rt_tick_get()//system_ms_tick
	
	#define XDEBUG
	#define CFG_LOG
	
#endif

#include "typedef.h"	
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Dump.h"
#include "_Macro.h"
#include "Printf.h"
#include "Bit.h"

#define IP_ADDR 		 "ehd.immotor.com"
#define IP_PORT 		 9003

//活动标志定义，当该标志被置位时，设备不能进入睡眠。
typedef enum _ActiveFlag
{
	 AF_BLE = BIT_0	//蓝牙处于连接状态
	,AF_MCU_RESET = BIT_1	//MCU处于延时复位状态
	,AF_NVC = BIT_2			//NVC正在播放语音
	
	,AF_SIM = BIT_8			//SIM模组有命令没有发完
	,AF_CARBIN_LOCK = BIT_9	//正在操作座舱锁
	
	,AF_PMS = BIT_16			//PMS
	,AF_BEACON = BIT_17			//BEACON是否连接状态
	
	,AF_FW_UPGRADE = BIT_24	//正在启动升级操作
}ActiveFlag;

//复位原因
typedef enum _MCURST
{
	 MCURST_PWR				//
	,MCURST_MODIFY_FW_VER	//修改固件版本号
	,MCURST_MODIFY_HW_VER	//修改硬件版本号
	,MCURST_MODIFY_SVR_ADDR	//修改服务器地址
	,MCURST_FWUPG_ERROR		//固件升级失败
	,MCURST_GPRS			//网络远程复位
	,MCURST_UPG_OK			//升级固件成功

	,MCURST_BLE_CMD			//收到蓝牙复位命令
	,MCURST_FACTORY_SETTING	//恢复出厂设置
	,MCURST_18650_POWER_OFF	//切断18650供电
	,MCURST_DEV_ACTIVE		//设备激活
	,MCURST_ALL_BAT_PLUG_OUT//拔出所有电池
	,MCURST_SMS				//短息复位
	,MCURST_PMS_FW_UPGRADE	//pms固件升级成功
	 
	,MCURST_ADC_ISR_INVALID	//ADC中断故障，不能触发
	,MCURST_QUEUE_FULL		//消息队列满
	,MCURST_GYRO			//陀螺仪唤醒
}MCURST;

typedef enum _WakeupType
{
	 WAKEUP_POWER = 0
	,WAKEUP_GYRO
	,WAKEUP_PMS
	,WAKEUP_SIM
	,WAKEUP_BLE
	,WAKEUP_RTC
	,WAKEUP_LOCK
#ifdef CFG_CABIN_LOCK	
	,WAKEUP_CABIN_LOCK
#endif
	,WAKEUP_TAKE_APART
	,WAKEUP_MAX
}WakeupType;

//ms ticks of system, 
extern volatile unsigned int system_ms_tick;

#ifdef __cplusplus
}
#endif

#endif




