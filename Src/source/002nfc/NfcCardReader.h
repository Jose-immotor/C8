
#ifndef __NFC_CARD_READER__H_
#define __NFC_CARD_READER__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "queue.h"

	//����������
	typedef enum _NfcCardReaderMsg
	{
		CARD_READER_MSG_RUN = 0X01,		//����
		CARD_READER_MSG_TIMEOUT,		//��ʱ
		CARD_READER_MSG_SEND,			//��������
		CARD_READER_MSG_WAKEUP,			//����
		CARD_READER_MSG_SEARCH_PORT,	//��������,��Ϣ������CARD_READER_MSG_SEARCH_PORT(port)
	}NfcCardReaderMsg;

	//������ʱ��
	typedef enum _NfcCardReaderEvent
	{
		CARD_EVENT_SEARCH_SUCCESS = 0X01,//�ѿ��ɹ�
		CARD_EVENT_SEARCH_FAILED,		//�ѿ�ʧ��
		CARD_EVENT_RX_DATA_SUCCESS,		//���յ����ݳɹ����¼�������(port, rxBuf, rxLen)
		CARD_EVENT_RX_DATA_FAILED,		//���յ�����ʧ�ܣ�ʧ��ԭ����latestErr��
		CARD_EVENT_TX_DATA_SUCCESS,		//���յ����ݳɹ����¼�������(port, txBuf, txLen)
		CARD_EVENT_TX_DATA_FAILED,		//���յ�����ʧ�ܣ�ʧ��ԭ����latestErr��
		CARD_EVENT_SLEEP,				//����
		CARD_EVENT_WAKEUP,				//����
	}NfcCardReaderEvent;

	typedef enum _NfcCardReaderEventRc
	{
		CARD_EVENT_RC_SUCCESS = 0,	//�ɹ�
		CARD_EVENT_RC_FAILED,		//ʧ��
	}NfcCardReaderEventRc;

	//����������״̬
	typedef enum _NfcCardReaderStatus
	{
		NfcCardReaderStatus_sleep,		//�͹���
		NfcCardReaderStatus_trans,		//����
		NfcCardReaderStatus_unknown,	//��ʼֵ
	}NfcCardReaderStatus;

	struct _NfcCardReader;
	typedef NfcCardReaderEventRc(*NfcCardReader_EventFn)(void* pObj, NfcCardReaderEvent ev);
	typedef void (*NfcCardReader_FsmFn)(struct _NfcCardReader* pReader, uint8_t msgId, uint32_t param);

	typedef struct _NfcCardReader
	{
		uint8_t port;	//����ID����0��ʼ
		uint8_t searchCounter;	//����ͬһ�˿ڼ�����
		uint32_t searchTicks;	//�����˿�Ticks,���ڼ�������ͬһ�˿ڵ�ʱ����


		unsigned char picc_ats[14];

		NfcCardReaderStatus status;

		//״̬������ָ�룬��status����ָ���Ǹ�״̬������
		NfcCardReader_FsmFn Fsm;

		uint8_t rxBuf[128];	//���ջ�����
		uint32_t rxLen;		//���ջ��������ݳ���
		uint8_t txBuf[128];	//���ͻ�����
		uint32_t txLen;		//���ͻ��������ݳ���

		uint8 latestErr;

		//�������ݴ���ص�����
		NfcCardReader_EventFn Event;
		//�ص��������󣬱��ص�����ʹ�ã����Գ�ʼ������
		void* cbObj;
	}NfcCardReader;

	void NfcCardReader_init(NfcCardReader* pReader, NfcCardReader_EventFn Event, void* cbObj);
	Bool NfcCardReader_isIdle(NfcCardReader* pReader);
	Bool NfcCardReader_Send(NfcCardReader* pReader, uint8_t port, const void* data, int len);
	void NfcCardReader_run(NfcCardReader* pReader);
	void NfcCardReader_start(NfcCardReader* pReader);
#ifdef __cplusplus
}
#endif

#endif

