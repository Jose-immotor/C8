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

	//��������ṹ����
	typedef struct _TransParam
	{
		/*===============���±������ⲿӦ��ʹ�ã��ⲿӦ���ڴ����ʼ��ʱ��������Event�ص�����������======================
		���ڷ������ݺͽ�������,
		����ʱ�����淢�����ݵ��ܳ��ȡ�
		����ʱ������������ݵ��ܳ��ȡ�
		****************************/
		int totalLen;		

		const uint8_t* txBuf;	//���ڷ��ͣ��������ݻ�����ָ��, �ⲿӦ�ÿ�����(TRANS_TX_BUF_EMPTY)�ص��������޸ġ�
		int txBufSize;			//���ڷ��ͣ��������ݻ���������, �ⲿӦ�ÿ�����EVENT(TRANS_TX_BUF_EMPTY)�ص��������޸ġ�

		uint8_t* rxBuf;			//���ڽ��գ��������ݻ�����ָ��, �ⲿӦ�ÿ�����EVENT(TRANS_RX_BUF_FULL)�ص��������޸�
		int rxBufSize;			//���ڽ��գ��������ݻ���������, �ⲿӦ�ÿ�����EVENT(TRANS_RX_BUF_FULL)�ص��������޸�

		/*===============���±����ɴ���Э��ʹ�ã��ⲿӦ�ò�����Event�ص��������޸�============================
		���ڷ��ͺͽ��գ���ʾ�Ѿ��ɹ����ͻ��߽��յ����ݳ���,
		****************************/
		int offset;
		/***************************
		���ڷ������ݺͽ�������,��Э�����ά�����ⲿӦ�ò�����Event�ص��������޸�
		����ʱ���Ѿ����͵�������txBuf�е�ƫ�ơ�
		����ʱ���Ѿ����յ�������txBuf�е�ƫ�ơ�
		****************************/
		int transBufOffset;		
		//���ڷ��ͣ���ǰ���ڴ�������ݰ����ȣ��ⲿӦ�ò�����Event�ص��������޸�
		int putBytesInTxFifo;	
	}TransParam;

	//����Э������
	typedef struct _TransProtocolCfg
	{
		uint8 fifoDeepth;	//Fifo����ȣ�������ֽ�����

	}TransProtocolCfg;

#ifdef __cplusplus
}
#endif

#endif


