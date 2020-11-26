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
		TRANS_RESULT_RX_BUF_SIZE_TOO_SMALL,	//接收缓冲区太小，不够保存一个数据包
		TRANS_RESULT_TX_BUF_SIZE_TOO_SMALL,	//接收缓冲区太小，不够保存一个数据包
	}TRANSFER_RESULT;

	typedef enum _TRANSFER_STATUS
	{
		TRANSFER_STATUS_IDLE = 0,	//空闲
		TRANSFER_STATUS_PENDING_TX,	//待发送
		TRANSFER_STATUS_TX,			//正在发送
		TRANSFER_STATUS_RX,			//正在接收
	}TRANSFER_STATUS;

	//传输类型定义
	typedef enum _TRANSFER_TYPE
	{
		TRANSFER_TYPE_TX,	//只有发送数据，不用接收数据
		TRANSFER_TYPE_TX_RX,//发送数据后在接收数据
	}TRANSFER_TYPE;

	typedef enum _TRANSFER_EVENT_RC
	{
		TRANS_EVT_RC_SUCCESS = 0,
		TRANS_EVT_RC_TIMEOUT,
	}TRANSFER_EVENT_RC;

	typedef enum _TRANS_EVENT
	{
		SEARCH_CARD_DONE,	//搜卡结束

		TRANS_TX_START,		//发送命令开始, 可在该事件中配置通信参数：waitRspMs, maxTxCount，transferData, transferLen

		TRANS_TX_BUF_EMPTY,	//应用于发送，发送的Buff空,应用层必须在该事件中写新数据进txBuf
		TRANS_RX_BUF_FULL,	//应用于接收，接收的Buff满,应用层必须在该事件中读走rxBuf的内容

		TRANS_FAILED,		//传输结束，失败，原因：可能传输超时，或者响应返回失败码。
		TRANS_SUCCESS,		//传输结束，成功，传输成功并且响应返回成功码
	}TRANS_EVENT;

	typedef TRANSFER_EVENT_RC (*TransEventFn)(void* obj, TRANS_EVENT evt);

	//传输参数结构定义
	typedef struct _TransParam
	{
		/*===============以下变量由外部应用使用，外部应用在传输初始化时，或者在Event回调函数中配置======================
		用于发送数据和接收数据,
		发送时：保存发送数据的总长度。
		接收时：保存接收数据的总长度。
		****************************/
		int totalLen;		

		const uint8_t* txBuf;	//用于发送，发送数据缓冲区指针, 外部应用可以在(TRANS_TX_BUF_EMPTY)回调函数中修改。
		int txBufSize;			//用于发送，发送数据缓冲区长度, 外部应用可以在EVENT(TRANS_TX_BUF_EMPTY)回调函数中修改。

		uint8_t* rxBuf;			//用于接收，接收数据缓冲区指针, 外部应用可以在EVENT(TRANS_RX_BUF_FULL)回调函数中修改
		int rxBufSize;			//用于接收，接收数据缓冲区长度, 外部应用可以在EVENT(TRANS_RX_BUF_FULL)回调函数中修改

		/*===============以下变量由传输协议使用，外部应用不能在Event回调函数中修改============================
		用于发送和接收，表示已经成功发送或者接收的数据长度,
		****************************/
		int offset;
		/***************************
		用于发送数据和接收数据,有协议代码维护，外部应用不能在Event回调函数中修改
		发送时：已经发送的数据在txBuf中的偏移。
		接收时：已经接收的数据在txBuf中的偏移。
		****************************/
		int transBufOffset;		
		//用于发送，当前正在传输的数据包长度，外部应用不能在Event回调函数中修改
		int putBytesInTxFifo;	
	}TransParam;

	//传输协议配置
	typedef struct _TransProtocolCfg
	{
		uint8 fifoDeepth;	//Fifo的深度（最大传输字节数）

	}TransProtocolCfg;

#ifdef __cplusplus
}
#endif

#endif


