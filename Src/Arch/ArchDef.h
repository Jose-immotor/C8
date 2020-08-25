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
	//#define Printf printf
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




