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

//��ز�
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
	//#define CFG_WHEEL_LOCK //������������ǵ����
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
#define COM_BUF_SIZE 512			//����Ҫ > 1024,��ֵ����
#define LEARN_MODE_MAX_SPEED 6		//ѧϰģʽ���������
#define LEARN_MODE_MAX_TRIP  1000	//ѧϰģʽ�������� 


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


//���־���壬���ñ�־����λʱ���豸���ܽ���˯�ߡ�
typedef enum _ActiveFlag
{
	 AF_BLE = BIT_0	//������������״̬
	,AF_MCU_RESET = BIT_1	//MCU������ʱ��λ״̬
	,AF_NVC = BIT_2			//NVC���ڲ�������
	
	,AF_SIM = BIT_8			//SIMģ��������û�з���
	,AF_CARBIN_LOCK = BIT_9	//���ڲ���������
	
	,AF_PMS = BIT_16			//PMS
	,AF_BEACON = BIT_17			//BEACON�Ƿ�����״̬
	
	,AF_FW_UPGRADE = BIT_24	//����������������
}ActiveFlag;

//��λԭ��
typedef enum _MCURST
{
	 MCURST_PWR				//
	,MCURST_MODIFY_FW_VER	//�޸Ĺ̼��汾��
	,MCURST_MODIFY_HW_VER	//�޸�Ӳ���汾��
	,MCURST_MODIFY_SVR_ADDR	//�޸ķ�������ַ
	,MCURST_FWUPG_ERROR		//�̼�����ʧ��
	,MCURST_GPRS			//����Զ�̸�λ
	,MCURST_UPG_OK			//�����̼��ɹ�

	,MCURST_BLE_CMD			//�յ�������λ����
	,MCURST_FACTORY_SETTING	//�ָ���������
	,MCURST_18650_POWER_OFF	//�ж�18650����
	,MCURST_DEV_ACTIVE		//�豸����
	,MCURST_ALL_BAT_PLUG_OUT//�γ����е��
	,MCURST_SMS				//��Ϣ��λ
	,MCURST_PMS_FW_UPGRADE	//pms�̼������ɹ�
	 
	,MCURST_ADC_ISR_INVALID	//ADC�жϹ��ϣ����ܴ���
	,MCURST_QUEUE_FULL		//��Ϣ������
	,MCURST_GYRO			//�����ǻ���
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
