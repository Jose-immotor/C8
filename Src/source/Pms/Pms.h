
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
#define PMS_ACC_OFF_ACTIVE_TIME 	(30*1000)//30��
#define PMS_ACC_DEEPSLEEP_TIME 		(10*1000)//5����

//
#define	PMS_LOW_CURRENT_2A				(200)	// ʵ��Ϊ 2A

#define	PMS_ACC_OFF_TIME				(5*60*1000)		// �ŵ����С��2A���� 5���ӣ���ر�����ģ��
#define	PMS_ACC_OFF_MODE_WAKUP_TIME		(10*60*1000)//(12*60*60*1000) //	// 12Сʱ�󣬻�������ģ��
#define	PMS_ACC_DEPSLEEP_TIME			(15*60*1000)//(48*60*60*1000)	//  // �ŵ����С��2A���� 2��,�رյ��,����֮



typedef enum
{
	_JT808_EXT_SLEEP = 0,
	_JT808_EXT_BRIEF_WAKUP,	// ��ʱ���� 12Сʱ
	_JT808_EXT_WAKUP,		// MCU��������ģ��
	_JT808_EXT_WAKUP_IRQ	// ����ģ���Լ�����
}JT808ExtStatus;


	//��ز���״̬
	typedef enum _PmsOpStatus
	{
		PMS_ACC_OFF = 1,	//Ϩ��,��ʼ״̬
		PMS_ACC_ON	,		//���
		PMS_SLEEP	,		//ǳ����
		PMS_DEEP_SLEEP,		//�������
		PMS_CAN_ERR,		//CANͨ�Ŵ���
	}PmsOpStatus;

	//PMS��Ϣ����
	typedef enum _PmsMsg
	{
		PmsMsg_run = 1,		//���У���Ϣ��ʽ��(BmsMsg_run, 0, 0)
		PmsMsg_timeout,		//��ʱ����Ϣ��ʽ��(BmsMsg_timeout, timerID, 0)
		
		PmsMsg_batPlugIn,	//��ز��룬��Ϣ��ʽ��(BmsMsg_batPlugIn, port, 0)
		PmsMsg_batPlugOut,	//��ذγ�����Ϣ��ʽ��(BmsMsg_batPlugout, port, 0)

		PmsMsg_accOn,	//��ز��룬��Ϣ��ʽ��(BmsMsg_batPlugIn, 0, 0)
		PmsMsg_accOff,	//��ذγ�����Ϣ��ʽ��(BmsMsg_batPlugout0, 0)

		PmsMsg_sleep,		//���ǳ���ߣ���Ϣ��ʽ��(BmsMsg_sleep, 0, 0)
		PmsMsg_deepSleep,	//��������ߣ���Ϣ��ʽ��(BmsMsg_deepSleep, 0, 0)
		PmsMsg_wakeup,		//��ػ��ѣ���Ϣ��ʽ��(BmsMsg_wakeup, 0, 0)
		
		PmsMsg_GyroIrq,		//�������𶯣���Ϣ��ʽ��(PmsMsg_GyroIrq, 0, 0)
#ifdef CANBUS_MODE_JT808_ENABLE	
		PmsMsg_GPRSIrq,		// GPRS�ж�
		PmsMsg_GPRSPlugIn,
		pmsMsg_GPRSPlugOut,
#endif
		PmsMsg_18650Low,	// 18650 �͵�
		PmsMsg_18650Normal, // 18650 ��������
	}PmsMsg;

	struct _Pms;
	typedef void (*Pms_FsmFn)(PmsMsg msgId, uint32_t param1, uint32_t param2);
	typedef struct _Pms
	{
		Mod modBus;
		Fm175Drv fmDrv;	//NFC��������

		//Pms����״̬
		PmsOpStatus opStatus;
		//״̬������ָ�룬��status����ָ���Ǹ�״̬������
		Pms_FsmFn Fsm;
		uint32_t statusSwitchTicks;

		//��Ϣ����
		Message msgBuf[32];
		Queue msgQueue;
	}Pms;

	extern Mod* g_pModBus;
	extern Battery g_Bat[];

	//�����߽��յ������ݣ����øú�������
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


