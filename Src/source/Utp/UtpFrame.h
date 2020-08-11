#ifndef __UTP_FRAME_H_
#define __UTP_FRAME_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "SwTimer.h"
#include "queue.h"
#include "UtpDef.h"

#define UTP_ENDLESS 0xFF

//接收数据包间隔
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
#define UTP_MIN_LEN 	6	//不包含帧头和帧尾

typedef int (*UtpTxFn)(const uint8_t* pData, int len);
typedef void (*UtpCmdTransferCb)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);

typedef struct _UtpPkt
{
	uint8_t m_state : 3;
	uint8_t m_Reserved : 5;

	uint8_t m_FrameState;
	uint16_t m_ReqLen;
	uint8_t m_Req[UTP_REQ_BUF_SIZE];	//存放发送出去数据，转码前
	uint8_t m_Rsp[UTP_RSP_BUF_SIZE];	//存放接收到的数据，转码前

	int   m_frameLen;
	uint8_t m_Frame[UTP_RSP_BUF_SIZE];//存放接收到的数据，m_Rsp转码后的帧数据
	Queue m_rxQueue;
}UtpPkt;

//typedef struct _Utp
//{
//	uint8_t m_state:3; 
//	uint8_t m_Reserved:5; 
//
//	uint8_t m_FrameState;
//	uint16_t m_ReqLen;
//	uint8_t m_Req[UTP_REQ_BUF_SIZE];	//存放发送出去数据，转码前
//	uint8_t m_Rsp[UTP_RSP_BUF_SIZE];	//存放接收到的数据，转码前
//
//	int   m_frameLen;
//	uint8_t m_Frame[UTP_RSP_BUF_SIZE];//存放接收到的数据，m_Rsp转码后的帧数据
//	Queue m_rxQueue;
//
//	uint8_t m_reTxCount;	//重发次数
//	uint8_t m_maxTxCount;	//最大重发次数
//	
//	UtpTxFn   TxFn;
//	int		m_cmdCount;			//事件数组总数
//	const UtpCmd* m_cmds;			//事件数组,
//	const UtpCmd* m_pCurrentCmd;	//当前的执行的命令，Null表示当前没有
//
//	//搜索帧头和帧尾变量
//	int    m_searchIndex;
//	Bool   m_isFoundHead;
//	//记录接收数据的当前Ticks，用于计算接收帧间隔
//	uint32_t m_rxDataTicks;
//
//	uint32_t m_rxRspTicks;			//接收响应的Ticks，用于计算发送帧间隔
//	uint32_t m_waitRspMs;			//等待响应时间
//	uint32_t m_sendCmdIntervalMs;	//发送命令间隔
//
//	SwTimer m_Timer;
//
//	UtpCmdTransferCb TresferEvent;	//事件回调函数
//	void* m_pCbObj;
//}Utp;

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


