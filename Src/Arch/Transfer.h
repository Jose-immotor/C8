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

	typedef struct _TransMgr
	{
		//用于发送和接收，发送时必须指明传输数据总长度,接收是可以为0，表示不指定传输数据总长度，实际的传输长度由offset记录
		int totalLen;		
		//用于发送和接收，表示已经成功发送或者接收的数据长度
		int offset;			

		int transBufLen;		//用于发送和接收，txBuf或rxBuf内的有效的长度
		int transBufOffset;		//用于发送和接收，已经传输的数据在txBuf/rxBuf中的偏移
		int transLen;			//用于发送，当前正在传输的数据包长度
	}TransMgr;

	//传输协议配置
	typedef struct _TransProtocolCfg
	{
		/*************************************************
		接收数据时：如果 FIFO 缓冲器空间剩下的字节数小于等于 WaterLevel 定义的字节数， 触发HiAlert中断
		发送数据时：如果 FIFO 中的字节数小于等于 WaterLevel 字节数， 触发LoAlert中断
		*************************************************/
		uint8 waterLevel;
		uint8 fifoDeepth;	//Fifo的深度（最大传输字节数）

	}TransProtocolCfg;

#ifdef __cplusplus
}
#endif

#endif


