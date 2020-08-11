#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif

#define SIM868

#define SUPPORTED_5V_DETECT
#define ALLOW_FW_UPGRADE_V 	3.7
#define PROTECT_18650_V 	3.3
#define X_MODEM
#define XDEBUG
#define CONFIG_CMDLINE

//#define PMS_IIC_ERROR_COUNT_ENABLE

//电池仓
	#define CFG_RENT

	#define GPRS_FW_UPGRADE
	#define CFG_LOGIN_TIME
	#define CFG_CYC_GET_BAT
	#define CFG_GET_PORT
	#define CFG_BLE_SLEEP
	#define CFG_GYRO 
	#define CFG_SIM_SLEEP

	#define CFG_LOG
	
	#define IP_ADDR_TEST 	 "119.23.133.72"
	#define IP_PORT_TEST 	 9003

	#define IP_ADDR 		 "ehd.immotor.com"
	#define IP_PORT 		 9003

	#define BLE_MF_STR 		 "30"
	#define IMMOTOR_KEY_WORD "IMT60"
	#define TIME_UPDATE_GPS_TO_SERVER 	(5*60*1000)
	#define BLE_DEFAULT_UUID_SSRC 	"FFF0"
	#define BLE_DEFAULT_UUID_SCHRR 	"FFF1"
	#define BLE_DEFAULT_UUID_SCHRT 	"FFF2"
	#define BLE_DEFAULT_PAIR_STATUS	  1
	#define BLE_DEFAULT_BIND_STATUS   1
	#define BLE_DEFAULT_PAIR_KEY 	"123456"
	
	#define BLE_BEACON_NAME "xBeacon"
	
	#define CFG_BEACON

	#define CFG_NVC
	#define CFG_CABIN_LOCK
	//#define CFG_WHEEL_LOCK //物理轮毂锁，非电机锁
	#define CFG_ASY
	#define CFG_REMOTE_ACC


#include <stdint.h>
#include <stdbool.h>
#include "typedef.h"

#include <string.h>
#include "archdef.h"
#include "commonDef.h"
#include "printf.h"
//#include "rtc.h"
#include "swtimer.h"
//#include "mx25_cmd.h"
//#include "sectorMgr.h"
//#include "Nvds.h"
//#include "smart_system.h"
//#include "page.h"
#include "math.h"
#include "msgdef.h"
//#include "GpioDef.h"
#include "queue.h"
#include "message.h"
//#include "CheckSum.h"
//#include "adc_sample.h"
//#include "errordef.h"
//#include "datatime.h"
#include "debug.h"
//#include "sign.h"
//#include "log.h"
//#include "bsp.h"

//#define ENABLE_POWER_KEY_LONG_CLICK 
#ifdef CFG_CABIN_LOCK
//#include "Cabin.h"
#endif

#define IS_FOUND(_str1, _str2) strstr((char*)(_str1), (char*)(_str2))
#define COM_BUF_SIZE 512			//必须要 > 1024,该值用于
#define LEARN_MODE_MAX_SPEED 6		//学习模式的最大限速
#define LEARN_MODE_MAX_TRIP  1000	//学习模式的最大里程 


extern volatile bool g_isPowerDown;


extern uint8_t g_CommonBuf[COM_BUF_SIZE];
extern uint8_t g_FirmwareVer[];

extern uint8_t g_isBleRun;
extern uint8_t g_isCtrlBoardRun;
extern uint8_t g_isMpuRun;
extern bool g_isMcuLowPower;
extern uint16_t g_AdcForBat;
extern uint16_t g_speed;
extern uint32_t g_TestFlag;

extern uint32 g_ResetMs;

#ifdef CONFIG_CMDLINE
	extern void thread_shell_entry(void* parameter);
#else
	#define Shell_Run(...)
#endif


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


extern void BootWithReason(MCURST reason);
extern void ResetDelay(MCURST reason, uint32_t ms);
extern void ResetStop(void);
extern void DelayPowerOff(uint32_t ms);

void Sys_Wakeup(void);
void Sys_Sleep(void);

int str_htoi(const char *s);
char* strstrex(const char* pSrc, const char* pDst);
int str_tok(char* strDst, const char* separator, int startInd, char* strArray[], int count);
char* strstr_remove(char* pSrc, const char* pDst, char**pValue);
int strtokenValueByIndex(char* strDst, const char* separator, int index);
//Bool Stream_IsIncludeTag(uint8_t* tag, int len, uint8_t byte);
//void DateTime_Add(S_RTC_TIME_DATA_T* dt, uint32_t second);
//void DateTime_dump(S_RTC_TIME_DATA_T* dt);
//int mem_mov(void* pDst, uint16_t dstLen, void* pSrc, uint16_t srcLen, int movLen);
//uint32_t SetMaskBits(uint32_t dst, uint32_t mask, uint32_t value);
void convertMac(const char* strMac, uint8_t* buff);
char* strcpyEx(char* pSrc, const char* pDst, const char* startStr, const char* endStr);
//int GetBitInd(uint32_t bitMask);
//Bool IsPrintChar(uint8_t byte);
int IsPktEnd(uint8_t byte, char tag, uint8_t* buf, int* ind, int buflen);
uint8_t* bytesSearch(const uint8_t* pSrc, int len, const char* pDst);
uint8_t* bytesSearchEx(const uint8_t* pSrc, int len, const char* pDst);
//char* ToBinStr(void* pByte, int len, char* str);
Bool hexStrToByteArray(const char* s, int len, uint8_t* pBuf);

#define strtoken(strDst, separator, strArray, count) str_tok(strDst, separator, 0, strArray, count)

extern void PostMsg(uint8_t msgId);
void Mcu_Delay(uint32_t ms);

#define PostMsgEx(...);

extern void  	  SetWakeUpType(WakeupType flag);
extern WakeupType  GetWakeUpType(void);
extern void Fsm_GoAlarmMode(Bool isEnable);
//extern void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);
extern void Boot(Bool isFromAppRom);

#ifdef __cplusplus
}
#endif

#endif
