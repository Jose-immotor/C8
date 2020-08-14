/******************************************************
UTP�����ڴ���Э�飬ʵ�֡������ն�������豸ͨѶЭ�顱�������͹��ܵ�Э�顣

��Э���ʺ����¸�ʽ�Ĵ���Э��
1 Э�����head�ֽں�Tail�ֽڣ�head�ֽں�Tail�ֽڿ�����ͬ��Ҳ���Բ�ͬ����frameCfg�ж���head��Tail��ֵ
2 Э���������������head�ֽں�Tail�ֽڣ�����Ҫת�룬��frameCfg����ת�����
3 Э�����һ��CMD�룬CMD���λ�ÿ�����֡ͷ����λ�ã���ѡ����SEQ��ADDR�������ֽڣ���frameCfg�ж�����麯������֡����
4 Э�����һ��CRC�룬CRC�����λ��PAYLOAD��ǰ����ߺ��档
5 �������Ӧ֡��DATA�ĵ�һ���ֽ�ΪRESULT�ֽ�
6 ֧������֡����
	1) READ:���������ͣ��������͵��ӻ��������߱���Ҫ��Ӧ.
	2��WRITE:д��������,�������͵��ӻ��������߱���Ҫ��Ӧ.
	3) EVENT���¼����ͣ��ӻ����͵������������߱���Ҫ��Ӧ��
	3��NOTIFY��֪ͨ����,˫�������Է��ͣ������߲���ҪӦ��

�����ʽ�������£�

          ֡ͷ    payload         crc
      |---------|---------------|----|
|-----|---------|---------------|----|----|
  head   *cmd*         data      crc  tail

**********************************************************/

#ifndef __UTP__H_
#define __UTP__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "SwTimer.h"
#include "queue.h"

//�ط���־�����޴��ط�
#define UTP_ENDLESS 0xFF

/***************
�����¼�����
***************/
typedef enum
{
	UTP_TX_START,		//�������ʼ, ���ڸ��¼�������ͨ�Ų�����waitRspMs, maxTxCount

	UTP_CHANGED_BEFORE,	//pStorageֵ�ı�֮ǰ
	UTP_CHANGED_AFTER,	//pStorageֵ�ı�֮��
	UTP_GET_RSP,		//���յ�����,��ȡ��Ӧ����Ӧ������transferData, transferLen��

	UTP_REQ_FAILED,		//���������ʧ�ܣ�ԭ�򣺿��ܴ��䳬ʱ��������Ӧ����ʧ���롣
	UTP_REQ_SUCCESS,	//����������ɹ�������ɹ�������Ӧ���سɹ���
}UTP_TXF_EVENT;

typedef enum
{
	UTP_EVENT_RC_SUCCESS = 0,	//�¼�����ɹ�
	UTP_EVENT_RC_FAILED,	//�¼�����ʧ��
	UTP_EVENT_RC_DO_NOTHING,	//�¼�û�д���
}UTP_EVENT_RC;

//UTP ����״̬
typedef enum
{
	UTP_FSM_INIT = 0	//��ʼ��
	, UTP_FSM_RX_REQ	//���յ�����
	, UTP_FSM_WAIT_RSP	//�ȴ���Ӧ
}UTP_FSM;

typedef enum
{
	UTP_READ = 0	//���������ͣ���ҪӦ��
	, UTP_WRITE		//д�������ͣ���ҪӦ��
	, UTP_EVENT		//�¼�����  ����ҪӦ��
	, UTP_NOTIFY	//֪ͨ����  ������ҪӦ��
}UTP_CMD_TYPE;

typedef enum _UTP_RCV_RSP_RC
{
	RSP_SUCCESS = 0
	, RSP_TIMEOUT
	, RSP_FAILED
	, RSP_CANCEL		//ȡ������
}UTP_RCV_RSP_RC;

//frame state;
typedef enum _FRAME_STATE
{
	FRAME_INIT = 0
	, FRAME_FOUND_HEAD
	, FRAME_FOUND_ESCAPE
	, FRAME_DONE
}FRAME_STATE;

//�������Ͷ���
struct _UtpCmd;
struct _Utp;
typedef UTP_EVENT_RC(*UtpEventFn)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);
typedef int (*UtpTxFn)(const uint8_t* pData, int len);

/*********************************
�������ܣ�֡У�飬ʵ��2�����ܣ�
	1�����req=Null,����������յ��������Ƿ�������
	2�����req!=Null,������յ�����֡�Ƿ��Reqƥ�䡣
	3�����Э���а���Seq�ֽں�Addr�ֽڣ�Ҳ����ʹ�øú�������͹���
����˵����
	pUtp��UTP����ָ�롣
	rxFrame��Ҫ���������֡��
	frameLen��֡���ȡ�
	req������֡�����req=Null����ʾ����Ҫ����Req
����ֵ��У������
***********************************/
typedef Bool(*UtpFrameVerifyFn)(struct _Utp* pUtp, const uint8_t* rxFrame, int frameLen, const uint8_t* req);

/*********************************
�������ܣ���Ҫ���͵��������ݴ����Ϊһ����������֡������Ӧ֡���ɲ���reqFrame������
����˵����
	pUtp��UTP����ָ�롣
	cmd�������롣
	pData������ָ�롣
	len�����ݳ���
	reqFrame�����=NULL����ʾ��������֡�����Ӧ֡�����!=NULL����ʾ���һ������֡��
	rspFrame�����������Ҫ���������ָ֡��
����ֵ������֡���ȡ�
***********************************/
typedef int (*UtpBuildFrameFn)(struct _Utp* pUtp, uint8_t cmd, const void* pData, int len, const uint8* reqFrame, uint8* rspFrame);

//Utp֡��������
typedef struct _UtpFrameCfg
{
	uint8_t head;		//֡ͷ
	uint8_t tail;		//֡β
	uint8_t transcode;		//ת����
	uint8_t transcodeHead;	//֡ͷת����
	uint8_t transcodeTail;	//֡βת����
	uint8_t transcodeShift;	//ת����-ת��

	uint8_t cmdByteInd;		//�����ֽ�ƫ��
	uint8_t dataByteInd;	//�����ֽ�ƫ��

	int      txBufLen;	//txBuf�ĳ���
	uint8_t* txBuf;		//��ŷ��ͳ�ȥ���ݣ�ת��ǰ
	int      rxBufLen;	//rxBuf�ĳ���
	uint8_t* rxBuf;		//��Ž��յ���RAW���ݣ�ת��ǰ
	int		 transcodeBufLen;//transcodeBuf�ĳ���
	uint8_t* transcodeBuf;	 //���rxBufת����֡����

	uint8_t result_SUCCESS;		//�����붨�壺�ɹ�
	uint8_t result_UNSUPPORTED;	//�����붨�壺���յ���֧�ֵ�����

	uint32_t waitRspMsDefault;	//�����Ĭ�ϵĵȴ���Ӧʱ�䣬�������Ҫ�޸�Ϊ��Ĭ��ֵ��������������¼�����UTP_TX_START���޸�pUtp->waitRspMs
	uint32_t rxIntervalMs;		//�������ݼ��
	uint32_t sendCmdIntervalMs;	//����2������֮��ļ��ʱ��

	UtpFrameVerifyFn FrameVerify;	//֡У�麯��
	UtpBuildFrameFn	 FrameBuild;	//֡�������
}UtpFrameCfg;

//Utp����������
typedef struct _UtpCfg
{

	int					  cmdCount;	//������������
	const struct _UtpCmd* cmdArray;	//��������

	UtpTxFn		TxFn;				//���ݵķ��͵����ߵĺ���ָ��
	void* pCbObj;				//�ص���������
	UtpEventFn	TresferEvent;		//�¼��ص�����
}UtpCfg;

typedef struct _UtpCmdEx
{
	/*
	�����EVENT��
		��UTP_GET_RSP�¼�֮ǰtransferDataָ����յ�������
		��UTP_GET_RSP�¼���transferDataĬ��ָ��UtpCmd->pData��Ҳ����
			���¼��������޸ģ�ָ��ҪӦ�������ָ�룬���ϲ�������
	�����READ/WRITE,ָ��Ӧ������
	*/
	const uint8_t* transferData;	//��������ָ��
	uint8_t transferLen;		//�������ݳ���

	/*****************
	��������������Ӧʱ��ʱ���Ticks.
	����UTP_READ/UTP_WRITE���ͣ����������Ӧʱ��Ticks��
	����UTP_EVENT���ͣ���Ч
	����UTP_NOTIFY���ͣ���������ʱ��Ticks��
	******************/
	uint32_t rxRspTicks;

	/*****************
	���ͱ�־:
		0����ʾ�����ͣ�
		����ֵ����ʾ��ʱָ����ʱ��MS����
	******************/
	uint32_t sendDelayMs;

	UTP_RCV_RSP_RC rcvRspErr;	//������Ӧ������
}UtpCmdEx;

typedef struct _UtpCmd
{
	UtpCmdEx* pExt;	//UtpCmd��չ

	UTP_CMD_TYPE type;	//������д����ο�UTP_CMD_TYPE����
	uint8_t cmd;			//������
	const char* cmdName;//��������

	//�����READ�����������������
	//�����WRITE������Ҫ���͵�д�������
	//�����EVENT������Event������������
	//�����NOTIFY����ʾ���Ͳ���
	//���ֵΪNull,��ʾ��ֵ������
	uint8_t* pStorage;
	int storageLen;

	//�����READ��pDataָ��Ҫ���͵Ķ��������
	//�����WRITE��pDataָ���Ѿ����ͳɹ������ݣ����ں�pStorage�Ƚ��Ƿ����仯��ȷ���Ƿ���Ҫ�����µ�д����
	//�����EVENT��pDataָ���¼���Ӧ��������ָ��
	//�����NOTIFY����,pData=Null
	//���ֵΪNull,��ʾ��ֵ������
	uint8_t* pData;
	int dataLen;

	UtpEventFn Event;	//�¼��ص�����
}UtpCmd;

typedef struct _Utp
{
	UTP_FSM state;
	FRAME_STATE FrameState;

	uint16_t txBufLen;		//����֡����
	uint16_t transcodeBufLen;	//ת����֡����
	Queue rxBufQueue;		//����rxBuf��Queue��

	//����֡ͷ��֡β����
	int    searchIndex;	//����ƫ��
	Bool   headIndex;		//֡ͷƫ��
	uint8_t*   head;		//֡ͷ,ָ��֡ͷ����

	//��¼��������֡�ĵ�ǰTicks�����ڼ������һ������֡��2������֮���Ƿ�ʱ
	uint32_t rxDataTicks;

	/*****************************************************************/
	uint8_t reTxCount;	//�ط�����
	uint8_t maxTxCount;	//����ط�����

	uint32_t rxRspTicks;			//������Ӧ��Ticks�����ڼ��㷢��֡���

	uint32_t waitRspMs;			//�ȴ���Ӧʱ��
	SwTimer waitRspTimer;		//�ȴ���Ӧ��ʱ��

	const UtpCmd* pWaitRspCmd;	//��ǰ��ִ�е��������ڵȴ�RSP�����������READ/WRITE��Null��ʾû��

	/*****************************************************************/
	const UtpCfg* cfg;
	const UtpFrameCfg* frameCfg;
}Utp;

void Utp_Init(Utp* pUtp, const UtpCfg* cfg, const UtpFrameCfg* frameCfg);
void Utp_Run(Utp* pUtp);

/************************************
�������ܣ�Э�����λ��
	ȡ���Ѿ��������ڵȴ���Ӧ�����
	ȡ�����еȴ����͵�����
	���Rx��������Req������
����˵����
	pUtp��Utp����
����ֵ��
***************************************/
void Utp_Reset(Utp* pUtp);

/************************************
�������ܣ������߻����ж��н������ݣ����浽���ջ�����rxBufQueue������
����˵����
	pUtp��Utp����
	pData�������߻����ж��н��յ������ݡ�
	len�����ݳ��ȡ�
����ֵ��
***************************************/
void Utp_RxData(Utp* pUtp, uint8_t* pData, int len);


/*******************************************
�������ܣ��������
����˵����
	pUtp��Utp����
	cmd�����͵������롣
����ֵ����
*******************************************/
void Utp_SendCmd(Utp* pUtp, uint8_t cmd);

/*******************************************
�������ܣ���ʱ�������
����˵����
	pUtp��Utp����
	cmd�����͵������롣
	delayMs����ʱ�����¼���0��ʾ����ʱ����������
����ֵ����
*******************************************/
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs);

#ifdef __cplusplus
}
#endif

#endif


