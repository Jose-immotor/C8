/******************************************************
���ɶ�д��Ȧ״̬0�£�0x01)
REQ: [��ַ][������:01][Addr-MSB][Addr-LSB][����������MSB][����������LSB][CRC��][CRC��]
RSP: [��ַ][������:01][�ֽ���][ ����][CRC��][CRC��]
EEE��[��ַ][�����:81][�쳣��]

�����ּĴ���(0x03)
REQ: [��ַ][������:03][Addr-MSB][Addr-LSB][�Ĵ�������MSB][�Ĵ�������LSB][CRC��][CRC��]
RSP: [��ַ][������:03][�ֽ���][�Ĵ���ֵ][CRC��][CRC��]
EEE��[��ַ][�����:83][�쳣��]

д�����Ĵ���(0x06)
REQ: [��ַ][������:06][Addr-MSB][Addr-LSB][�Ĵ���ֵ(2 Bytes)][CRC��][CRC��]
RSP: [��ַ][������:06][Addr-MSB][Addr-LSB][�Ĵ���ֵ(2 Bytes)][CRC��][CRC��]
EEE��[��ַ][�����:86][�쳣��]
**********************************************************/

#ifndef __MODBUS__H_
#define __MODBUS__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "SwTimer.h"

	//�ط���־�����޴��ط�
#define MOD_ENDLESS 0xFF

#define MODBUS_CMD_IND  4
#define MODBUS_LEN_IND  5
#define MODBUS_DATA_IND 6
//#define MODBUS_READDATA_IND 6

	typedef enum
	{
		MOD_ERR_ILLEG_FUN  = 0X01,	//�Ƿ�����
		MOD_ERR_ILLEG_ADDR = 0X02,	//�Ƿ���ַ
	}MOD_ERR;

	//���ߴ�����
	typedef enum
	{
		BUS_ERR_OK  = 0X00,	//�Ƿ����� lane 0x01->0x00
		BUS_ERR_TX_FAILED,
		BUS_ERR_RX_FAILED,
	}BUS_ERR;

	typedef enum
	{
		MOD_READ_COIL_STATUS = 0X01,	//����Ȧ״̬,��
		MOD_READ_INPUT_STATUS = 0x02,		//����ɢ����Ĵ���
		MOD_READ_HOLDING_REG = 0x03,		//�����ּĴ���
		MOD_READ_INPUT_REG = 0x04,		//������Ĵ���
		MOD_WEITE_SINGLE_COIL = 0x05,		//д������Ȧ
		MOD_WEITE_SINGLE_REG = 0x06,		//д�����Ĵ���
		MOD_READ_EXCEPTION_STATUS = 0x07, //���쳣״̬

		MOD_WEITE_MULTI_COIL = 0x0F,	//д�����Ȧ
		MOD_WEITE_MULTI_REG = 0x10,			//д����Ĵ���
		
	}MOD_CMD;

	/***************
	�����¼�����
	***************/
	typedef enum _MOD_TXF_EVENT
	{
		MOD_TX_START = 0	//���͹����뿪ʼ, ���ڸ��¼�������ͨ�Ų�����waitRspMs, maxTxCount
		, MOD_CHANGED_BEFORE	//pStorageֵ�ı�֮ǰ
		, MOD_CHANGED_AFTER	//pStorageֵ�ı�֮��
		, MOD_GET_RSP		//���յ�����,��ȡ��Ӧ����Ӧ������transferData, transferLen��
		, MOD_REQ_FAILED		//���������ʧ�ܣ�ԭ�򣺿��ܴ��䳬ʱ��������Ӧ����ʧ���롣
		, MOD_REQ_SUCCESS	//����������ɹ�������ɹ�������Ӧ���سɹ���
	}MOD_TXF_EVENT;

	typedef enum
	{
		MOD_EVENT_RC_SUCCESS = 0	//�¼�����ɹ�
		, MOD_EVENT_RC_FAILED		//�¼�����ʧ��
		, MOD_EVENT_RC_DO_NOTHING	//�¼�û�д���
	}MOD_EVENT_RC;

	//MOD ����״̬
	typedef enum
	{
		MOD_FSM_INIT = 0	//��ʼ��
		, MOD_FSM_RX_REQ	//���յ�����
		, MOD_FSM_WAIT_RSP	//�ȴ���Ӧ
	}MOD_FSM;

	typedef enum
	{
		MOD_READ = 0	//�����������ͣ���ҪӦ��
		, MOD_WRITE		//д���������ͣ���ҪӦ��
		, MOD_NOTIFY	//֪ͨ����  ������ҪӦ��
	}MOD_CMD_TYPE;

	typedef enum _MOD_RSP_RC
	{
		MOD_RSP_SUCCESS = 0		//������Ӧ�ɹ�
		, MOD_RSP_TIMEOUT		//������Ӧ��ʱ���Է�û��Ӧ
		, MOD_TRANS_FAILED		//����ʧ�ܣ��������ߴ��󣬴�������busErr�м�¼
		, MOD_RSP_CANCEL		//ȡ������
	}MOD_RSP_RC;

	//�������Ͷ���
	struct _ModCmd;
	struct _Mod;
	typedef MOD_EVENT_RC(*ModEventFn)(void* pOwnerObj, const struct _ModCmd* pCmd, MOD_TXF_EVENT ev);
	typedef int (*ModTxFn)(const uint8_t* pData, int len);

	//Mod֡��������
	typedef struct _ModFrame
	{
		uint8_t  addr;	//txBuf�ĳ���
		uint8_t  cmd;		//��ŷ��ͳ�ȥ���ݣ�ת��ǰ
		int      rxBufLen;	//rxBuf�ĳ���
		uint8_t* rxBuf;		//��Ž��յ���RAW���ݣ�ת��ǰ

		uint32_t waitRspMsDefault;	//�������Ĭ�ϵĵȴ���Ӧʱ�䣬���������Ҫ�޸�Ϊ��Ĭ��ֵ�������ڹ�������¼�����MOD_TX_START���޸�pMod->waitRspMs
		uint32_t rxIntervalMs;		//�������ݼ��
		uint32_t sendCmdIntervalMs;	//����2��������֮��ļ��ʱ��
	}ModFrame;


	/*********************************
	�������ܣ�֡У�飬ʵ��2�����ܣ�
		1�����req=Null,����������յ��������Ƿ�������
		2�����req!=Null,������յ�����֡�Ƿ��Reqƥ�䡣
		3�����Э���а���Seq�ֽں�Addr�ֽڣ�Ҳ����ʹ�øú�������͹���
	����˵����
		pMod��MOD����ָ�롣
		rxFrame��Ҫ���������֡��
		frameLen��֡���ȡ�
		req������֡�����req=Null����ʾ����Ҫ����Req
	����ֵ��У������
	***********************************/
	typedef Bool(*ModFrameVerifyFn)(struct _Mod* pMod, const uint8_t* rxFrame, int frameLen, const uint8_t* req);

	/*********************************
	�������ܣ���Ҫ���͵��������ݴ����Ϊһ����������֡������Ӧ֡���ɲ���reqFrame������
	����˵����
		pMod��MOD����ָ�롣
		cmd���������롣
		pData������ָ�롣
		len�����ݳ���
		reqFrame�����=NULL����ʾ��������֡�����Ӧ֡�����!=NULL����ʾ���һ������֡��
		rspFrame�����������Ҫ���������ָ֡��
	����ֵ������֡���ȡ�
	***********************************/
	typedef int (*ModBuildFrameFn)(struct _Mod* pMod, uint8_t cmd, const void* pData, int len, const uint8* reqFrame, uint8* rspFrame);

	//Mod֡��������
	typedef struct _ModFrameCfg
	{
		int      txBufLen;	//txBuf�ĳ���
		uint8_t* txBuf;		//��ŷ��ͳ�ȥ���ݣ�ת��ǰ
		int      rxBufLen;	//rxBuf�ĳ���
		uint8_t* rxBuf;		//��Ž��յ���RAW���ݣ�ת��ǰ

		uint32_t waitRspMsDefault;	//�������Ĭ�ϵĵȴ���Ӧʱ�䣬���������Ҫ�޸�Ϊ��Ĭ��ֵ�������ڹ�������¼�����MOD_TX_START���޸�pMod->waitRspMs
		uint32_t rxIntervalMs;		//�������ݼ��
		uint32_t sendCmdIntervalMs;	//����2��������֮��ļ��ʱ��
		//
		
	}ModFrameCfg;

	//Mod����������
	typedef struct _ModCfg
	{
		uint8_t  port;	//��ض˿ںã���0��ʼ
		uint8_t  antselct;	//����ѡ��
		int					  cmdCount;	//��������������
		const struct _ModCmd* cmdArray;	//����������

		ModTxFn		TxFn;				//���ݵķ��͵����ߵĺ���ָ��
		void* pCbObj;				//�ص���������
		ModEventFn	TresferEvent;		//�¼��ص�����
	}ModCfg;

	typedef struct _ModCmdFmt
	{
		uint8_t dataByteInd;
		uint8_t lenByteInd;
		uint8_t regByteInd;
	}ModCmdFmt;

	typedef struct _ModCmdEx
	{
		/*
		�����EVENT��
			��MOD_GET_RSP�¼�֮ǰtransferDataָ����յ�������
			��MOD_GET_RSP�¼���transferDataĬ��ָ��ModCmd->pData��Ҳ����
				���¼��������޸ģ�ָ��ҪӦ�������ָ�룬���ϲ�������
		�����READ/WRITE,ָ��Ӧ������
		*/
		const uint8_t* transferData;	//��������ָ��
		uint8_t transferLen;		//�������ݳ���

		/*****************
		��������������Ӧʱ��ʱ���Ticks.
		����MOD_READ/MOD_WRITE���ͣ����������Ӧʱ��Ticks��
		����MOD_EVENT���ͣ���Ч
		����MOD_NOTIFY���ͣ���������ʱ��Ticks��
		******************/
		uint32_t rxRspTicks;

		/*****************
		���ͱ�־:
			0����ʾ�����ͣ�
			����ֵ����ʾ��ʱָ����ʱ��MS����
		******************/
		uint32_t sendDelayMs;

		MOD_RSP_RC rcvRspErr;	//������Ӧ������

		Bool isForceSend;		//ǿ�Ʒ��ͱ�־
	}ModCmdEx;

	typedef struct _ModCmd
	{
		ModCmdEx* pExt;		//ModCmd��չ
		uint8_t cmd;		//������ID
		MOD_CMD_TYPE type;	//����������
		uint8_t modCmd;		//��������
		const char* cmdName;//����������

		//uint8_t regAddr[2];
		//�����READ����������������ݵļĴ�����ַ
		//�����WRITE������Ҫ���͵�д���������
		//���ֵΪNull,��ʾ��ֵ������
		void* pStorage;
		int storageLen;

		//�����READ����ΪpStorage��Mirror������pStorageͬʱ����READ��WRITE������ʱ����
		//�����WRITE����Ϊ������
		//�����READ����Ϊд����
		//���ֵΪNull,��ʾ��ֵ������
		void* pParam;
		uint8_t  paramLen;

		//pStorage��Mirror���������Ⱥͱ��뱣��pStorageһ�¡�
		//�����READ�������������ݸ���pStorage��Mirror
		//�����WRITE�����淢�ͳɹ������ݣ����ں�pStorage�Ƚ��Ƿ����仯��ȷ���Ƿ���Ҫ�����µ�д������
		//���ֵΪNull,��ʾ��ֵ������
		void* mirror;

		ModEventFn Event;	//�¼��ص�����
	}ModCmd;

	typedef struct _Mod
	{
		uint8_t addr;			//�豸��ַ
		MOD_FSM state;

		uint16_t txBufLen;		//����֡����
		uint16_t rxBufLen;		//���յĵ�֡����

		//��¼��������֡�ĵ�ǰTicks�����ڼ������һ������֡��2������֮���Ƿ�ʱ
		uint32_t rxDataTicks;
		BUS_ERR  busErr;	//���ߴ���

		/*****************************************************************/
		uint8_t reTxCount;	//�ط�����
		uint8_t maxTxCount;	//����ط�����

		uint32_t rxRspTicks;			//������Ӧ��Ticks�����ڼ��㷢��֡���

		uint32_t waitRspMs;			//�ȴ���Ӧʱ��
		SwTimer waitRspTimer;		//�ȴ���Ӧ��ʱ��

		const ModCmd* pWaitRspCmd;	//��ǰ��ִ�е��������ڵȴ�RSP�Ĺ����룬������READ/WRITE��Null��ʾû��

		/*****************************************************************/
		const ModCfg* cfg;
		const ModFrameCfg* frameCfg;
	}Mod;

	void Mod_Init(Mod* pMod, ModCfg* cfg, const ModFrameCfg* frameCfg);
	void Mod_Run(Mod* pMod);

	/************************************
	�������ܣ�Э�����λ��
		ȡ���Ѿ��������ڵȴ���Ӧ�Ĺ����룬
		ȡ�����еȴ����͵Ĺ�����
		���Rx��������Req������
	����˵����
		pMod��Mod����
	����ֵ��
	***************************************/
	void Mod_Reset(Mod* pMod);
	void Mod_ResetCmds(const ModCfg* cfg);

	/************************************
	�������ܣ������߻����ж��н������ݣ����浽���ջ�����rxBufQueue������
	����˵����
		pMod��Mod����
		pData�������߻����ж��н��յ������ݡ�
		len�����ݳ��ȡ�
	����ֵ��
	***************************************/
	void Mod_RxData(Mod* pMod, const uint8_t* pData, int len);


	/*******************************************
	�������ܣ����͹����롣
	����˵����
		pMod��Mod����
		cmd�����͵Ĺ������롣
	����ֵ����
	*******************************************/
	void Mod_SendCmd(const ModCfg* cfg, uint8_t cmd);

	/*******************************************
	�������ܣ���ʱ���͹����롣
	����˵����
		pMod��Mod����
		cmd�����͵Ĺ������롣
		delayMs����ʱ�����¼���0��ʾ����ʱ����������
	����ֵ����
	*******************************************/
	void Mod_DelaySendCmd(const ModCfg* cfg, uint8_t cmd, uint32_t delayMs);

	//����Э������Ƿ����
	Bool Mod_isIdle(Mod* pMod);
	Bool Mod_SwitchCfg(Mod* pMod, const ModCfg* cfg);
	const ModCmd* Mod_FindCmdItem(const ModCmd* pCmd, int count, uint8_t cmd);

	//���ߴ���
	void Mod_busErr(Mod* pMod, BUS_ERR err);

#ifdef __cplusplus
}
#endif

#endif


