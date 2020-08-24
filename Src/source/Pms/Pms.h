
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
#include "NfcCardReader.h"

	//��ز���״̬
	typedef enum _PmsOpStatus
	{
		PMS_ACC_OFF	,		//Ϩ��,��ʼ״̬
		PMS_ACC_ON	,		//���
		PMS_SLEEP	,		//ǳ����
		PMS_DEEP_SLEEP,	//�������
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
	}PmsMsg;

	struct _Pms;
	typedef void (*Pms_FsmFn)(uint8_t msgId, uint32_t param1, uint32_t param2);
	typedef struct _Pms
	{
		Mod modBus;

		NfcCardReader cardReader;

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

	//�����߽��յ������ݣ����øú�������
	void Pms_init();
	void Pms_Rx(int nfcPort, const uint8_t* pData, int len);
	void Pms_postMsg(uint8_t msgId, uint32_t param1, uint32_t param2);

#ifdef __cplusplus
}
#endif

#endif


