#ifndef __UTP__H_
#define __UTP__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "SwTimer.h"
#include "queue.h"
#include "UtpDef.h"

#define UTP_ENDLESS 0xFF

//�������ݰ����
#define UTP_RX_DATA_INTERVAL_MS 1000

//frame state;
typedef enum _FRAME_STATE
{
	FRAME_INIT = 0
	,FRAME_FOUND_HEAD
	,FRAME_FOUND_ESCAPE
	,FRAME_DONE
}FRAME_STATE;

typedef struct _UtpFrame
{
	uint8_t  checkSum;
	uint8_t  ver;
	uint16_t vendor;
	uint8_t  devType;
	uint8_t  cmd;
	uint8_t  data[1];
}UtpFrame;
#define UTP_HEAD_LEN 	6
#define UTP_MIN_LEN 	6	//������֡ͷ��֡β

typedef int (*UtpTxFn)(const uint8_t* pData, int len);
typedef void (*UtpCmdTransferCb)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);

typedef struct _Utp
{
	uint8_t m_state:3; 
	uint8_t m_Reserved:5; 

	uint8_t m_FrameState;

	uint16_t m_txBufLen;
	uint8_t m_txBuf[UTP_REQ_BUF_SIZE];	//��ŷ��ͳ�ȥ���ݣ�ת��ǰ
	uint8_t m_rxBuf[UTP_RSP_BUF_SIZE];	//��Ž��յ���RAW���ݣ�ת��ǰ
	Queue m_rxBufQueue;					//����m_rxBuf�ġ�

	int   m_frameLen;
	uint8_t m_Frame[UTP_RSP_BUF_SIZE];//��Ž��յ������ݣ�m_rxBufת����֡����

	//����֡ͷ��֡β����
	int    m_searchIndex;
	Bool   m_isFoundHead;

	//��¼��������֡�ĵ�ǰTicks�����ڼ������һ������֡��2������֮���Ƿ�ʱ
	uint32_t m_rxDataTicks;

	/*****************************************************************/
	uint8_t m_reTxCount;	//�ط�����
	uint8_t m_maxTxCount;	//����ط�����
	
	int		m_cmdCount;				//������������
	const UtpCmd* m_cmds;			//��������,
	const UtpCmd* m_pCurrentCmd;	//��ǰ��ִ�е����Null��ʾ��ǰû��

	uint32_t m_rxRspTicks;			//������Ӧ��Ticks�����ڼ��㷢��֡���
	uint32_t m_sendCmdIntervalMs;	//����������ʱ��

	uint32_t m_waitRspMs;			//�ȴ���Ӧʱ��
	SwTimer m_waitRspTimer;		//�ȴ���Ӧ��ʱ��

	UtpTxFn   TxFn;		//���ݵķ��͵����ߵĺ���ָ��
	UtpCmdTransferCb TresferEvent;	//�¼��ص�����
	void* m_pCbObj;
}Utp;

void Utp_Init(Utp* pUtp, UtpTxFn txFn, const UtpCmd* pCmds, int cmdCount, UtpCmdTransferCb evFn, void* pCbObj);
void Utp_Run(Utp* pUtp);
Bool Utp_isBusy(Utp* pUtp);
void Utp_Reset(Utp* pUtp);
void Utp_RxData(Utp* pUtp, uint8_t* pData, int len);
void Utp_SendCmd(Utp* pUtp, uint8_t cmd);
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs);

#ifdef __cplusplus
}
#endif

#endif


