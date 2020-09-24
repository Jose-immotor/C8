/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */
 
#ifndef __TRANSFER__H_
#define __TRANSFER__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

	typedef enum _FM_TX_CMD_RESULT
	{
		TRANS_RESULT_SUCCESS = 0,
		TRANS_RESULT_FAILED,
		TRANS_RESULT_TIMEOUT,
		TRANS_RESULT_SEARCH_CARD_FAILED,
		TRANS_RESULT_RX_BUF_SIZE_TOO_SMALL,	//���ջ�����̫С����������һ�����ݰ�
		TRANS_RESULT_TX_BUF_SIZE_TOO_SMALL,	//���ջ�����̫С����������һ�����ݰ�
	}TRANSFER_RESULT;

	typedef enum _TRANSFER_STATUS
	{
		TRANSFER_STATUS_IDLE = 0,	//����
		TRANSFER_STATUS_PENDING_TX,	//������
		TRANSFER_STATUS_TX,			//���ڷ���
		TRANSFER_STATUS_RX,			//���ڽ���
	}TRANSFER_STATUS;

	//�������Ͷ���
	typedef enum _TRANSFER_TYPE
	{
		TRANSFER_TYPE_TX,	//ֻ�з������ݣ����ý�������
		TRANSFER_TYPE_TX_RX,//�������ݺ��ڽ�������
	}TRANSFER_TYPE;

	typedef enum _TRANSFER_EVENT_RC
	{
		TRANS_EVT_RC_SUCCESS = 0,
		TRANS_EVT_RC_TIMEOUT,
	}TRANSFER_EVENT_RC;

	typedef enum _TRANS_EVENT
	{
		SEARCH_CARD_DONE,	//�ѿ�����

		TRANS_TX_START,		//�������ʼ, ���ڸ��¼�������ͨ�Ų�����waitRspMs, maxTxCount��transferData, transferLen

		TRANS_TX_BUF_EMPTY,	//Ӧ���ڷ��ͣ����͵�Buff��,Ӧ�ò�����ڸ��¼���д�����ݽ�txBuf
		TRANS_RX_BUF_FULL,	//Ӧ���ڽ��գ����յ�Buff��,Ӧ�ò�����ڸ��¼��ж���rxBuf������

		TRANS_FAILED,		//���������ʧ�ܣ�ԭ�򣺿��ܴ��䳬ʱ��������Ӧ����ʧ���롣
		TRANS_SUCCESS,		//����������ɹ�������ɹ�������Ӧ���سɹ���
	}TRANS_EVENT;

	typedef TRANSFER_EVENT_RC (*TransEventFn)(void* obj, TRANS_EVENT evt);

	typedef struct _TransMgr
	{
		//���ڷ��ͺͽ��գ�����ʱ����ָ�����������ܳ���,�����ǿ���Ϊ0����ʾ��ָ�����������ܳ��ȣ�ʵ�ʵĴ��䳤����offset��¼
		int totalLen;		
		//���ڷ��ͺͽ��գ���ʾ�Ѿ��ɹ����ͻ��߽��յ����ݳ���
		int offset;			

		int transBufLen;		//���ڷ��ͺͽ��գ�txBuf��rxBuf�ڵ���Ч�ĳ���
		int transBufOffset;		//���ڷ��ͺͽ��գ��Ѿ������������txBuf/rxBuf�е�ƫ��
<<<<<<< .mine
		int putBytesInTxFifo;		//���ڷ��ͣ���ǰ���ڴ�������ݰ�����
=======
		int putBytesInTxFifo;			//���ڷ��ͣ���ǰ���ڴ�������ݰ�����
>>>>>>> .theirs
	}TransMgr;

	//����Э������
	typedef struct _TransProtocolCfg
	{
		/*************************************************
		��������ʱ����� FIFO �������ռ�ʣ�µ��ֽ���С�ڵ��� WaterLevel ������ֽ����� ����HiAlert�ж�
		��������ʱ����� FIFO �е��ֽ���С�ڵ��� WaterLevel �ֽ����� ����LoAlert�ж�
		*************************************************/
		uint8 waterLevel;
		uint8 fifoDeepth;	//Fifo����ȣ�������ֽ�����

	}TransProtocolCfg;

#ifdef __cplusplus
}
#endif

#endif


