
#ifndef __BATTERY__H_
#define __BATTERY__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "BmsReg.h"
#include "ModBus.h"
#include "NfcCardReader.h"

#define BMS_INQUERY_INTERVAL_MS 1000	//��ѯ�¼����
#define NFC_MAX_TRANS_SIZE 		125		//NFCÿ�δ������������

#define BMS_REG_ID_SIZE  sizeof(BmsReg_info)
#define BMS_REG_ID_COUNT (sizeof(BmsReg_info) / 2)

#define BMS_REG_INFO_COUNT_1 ((BMS_REG_INFO_ADDR_2 - BMS_REG_INFO_ADDR_1))
#define BMS_REG_INFO_SIZE_1  (BMS_REG_INFO_COUNT_1 * 2)
#define BMS_REG_INFO_COUNT_2 (BMS_REG_INFO_END - BMS_REG_INFO_ADDR_2 + 1)
#define BMS_REG_INFO_SIZE_2  (BMS_REG_INFO_COUNT_2 * 2)

#define BMS_REG_CTRL_SIZE  sizeof(BmsReg_ctrl)
#define BMS_REG_CTRL_COUNT (sizeof(BmsReg_ctrl) / 2)

	/*
	typedef enum _BmsCmdCtrl
	{
		RegCtrl_chg_en = BIT_0,
		RegCtrl_chg_disEn  = BIT_1,
		RegCtrl_disChg_en  = BIT_2,
		RegCtrl_disChg_disEn = BIT_3,
		RegCtrl_preDischg_en 	= BIT_4,
		RegCtrl_preDischg_disEn  = BIT_5,
		RegCtrl_sleep_en 		= BIT_6,
		RegCtrl_sleep_disEn  	= BIT_7,
		RegCtrl_deepSleep_en 	= BIT_8,
		RegCtrl_deepSleep_disEn = BIT_9,
	}BmsCmdCtrl;
	
*/
	typedef enum _BmsCmd
	{
		BMS_READ_ID = 0X01,	//��BMS��ݰ汾�Ĵ���
		BMS_READ_INFO_1,	//��BMS��Ϣ�Ĵ���1, һ����̫�󣬷ֿ�2�ζ�ȡ��
		BMS_READ_INFO_2,	//��BMS��Ϣ�Ĵ���2
		BMS_READ_CTRL,		//��BMS���ƼĴ���

		BMS_WRITE_CTRL, 	//д���ƼĴ���
	}BmsCmd;

	//�����Ϣ����
	typedef enum _BmsMsg
	{
		BmsMsg_run = 1,		//���У���Ϣ��ʽ��(BmsMsg_run, 0, 0)
		BmsMsg_timeout,		//��ʱ����Ϣ��ʽ��(BmsMsg_timeout, timerID, 0)
		BmsMsg_cmdDone,		//�����������Ϣ��ʽ��(BmsMsg_cmdDone, cmd, MOD_EVENT_RC)

		BmsMsg_batPlugIn,	//��ز��룬��Ϣ��ʽ��(BmsMsg_batPlugIn, 0, 0)
		BmsMsg_batPlugout,	//��ذγ�����Ϣ��ʽ��(BmsMsg_batPlugout(0, 0)
		BmsMsg_active,		//��ؼ����Ϣ��ʽ��(BmsMsg_active, ctrl, 0)
		BmsMsg_deactive,	//���ȥ�����Ϣ��ʽ��(BmsMsg_deactive, 0, 0)

		BmsMsg_sleep,		//���ǳ���ߣ���Ϣ��ʽ��(BmsMsg_sleep, 0, 0)
		BmsMsg_deepSleep,	//��������ߣ���Ϣ��ʽ��(BmsMsg_deepSleep, 0, 0)
		BmsMsg_wakeup,		//��ػ��ѣ���Ϣ��ʽ��(BmsMsg_wakeup, 0, 0)
	}BmsMsg;

	//��ز���״̬
	typedef enum _BmsOpStatus
	{
		BmsStatus_init,		//��ʼ״̬
		BmsStatus_readInfo,	//��ѯ��أ�����л���Idle״̬
		BmsStatus_idle,		//���У��ȴ���һ����ѯ
		BmsStatus_sleep,	//ǳ���ߣ���ѯ���״̬
		BmsStatus_deepSleep,//������ߣ�����ѯ���״̬
	}BmsOpStatus;

	//�����λ״̬
	typedef enum _BatPresentStatus
	{
		BAT_UNKNOWN,	//��֪���Ƿ���λ��û�����ϣ�
		BAT_NOT_IN,	//��ز���λ
		BAT_IN,		//�����λ������ȡ������Ϣ��ϣ�
	}BatPresentStatus;

	typedef struct _Battery
	{
		NfcCardReader cardReader;

		uint8_t  port;					//��ض˿ںã���0��ʼ
		uint32_t idleTicks;				//ȥ����ʱ��Ticks
		uint32_t statusSwitchTicks;		//״̬�л�Ticks
		Bool 	 isActive;				//�Ƿ񼤻�

		Bool 	 isReadyFroInquery;	//�Ƿ�׼���ò�ѯ���

		BatPresentStatus presentStatus;	//�����λ״̬
		BmsOpStatus 	 opStatus;	//��ز���״̬

		//static data

		BmsReg_info 		bmsID;		//��Ϣ�Ĵ���
		BmsReg_deviceInfo 	bmsInfo;	//ֻ���Ĵ���
		BmsReg_ctrl 		bmsCtrl;	//���ƼĴ���

		//��Ҫд��BMS�Ĵ���BmsRegCtrl��ֵ
		uint16_t   			writeBmsCtrl;

		//bmsCtrl�������,��¼WIRTE�ɹ�ʱ��ֵ��
		//����Modbos����Э��ʹ�ã�����ģ�鲻���޸�
		uint16_t 			writeBmsCtrl_mirror;
		
		BmsRegCtrl* 		pBmsReg_Ctrl;	//��ʼ��ָ�� writeBmsCtrl;
		const ModCfg* cfg;
	}Battery;

	void Bat_init(Battery* pBat, int port, const ModCfg* cfg);
	void Bat_start(Battery* pBat);
	void Bat_run(Battery* pBat);
	const uint8* Bat_getBID(Battery* pBat);
	void Bat_dump(const Battery* pBat);
	void Bat_bmsInfoDump(const Battery* pBat);

	//�����Ϣ��Ч
	Bool Bat_isReady(Battery* pBat);

	//��Ϣ������
	void Bat_msg(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2);

	//��ؿ��ƺ���
	void Bat_setPreDischg(Battery* pBat, Bool en);
	void Bat_setDischg(Battery* pBat, Bool en);
	void Bat_setChg(Battery* pBat, Bool en);
	void Bat_setSleep(Battery* pBat, Bool en);
	void Bat_setDeepSleep(Battery* pBat, Bool en);


	//Bat �¼�����
	MOD_EVENT_RC Bat_event(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev);
	MOD_EVENT_RC Bat_event_readBmsInfo(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev);

#ifdef __cplusplus
}
#endif

#endif


