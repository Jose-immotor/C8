
#include "common.h"
#include "CheckSum.h"
#include "Utp.h"

#define UTP_PRINTF Printf

static Bool Utp_RspProc(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static void Utp_RcvRsp(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static OP_CODE Utp_RcvReq(Utp* pUtp, const UtpFrame* pReq, int frameLen);

/*
把Payload数据加上帧头和帧为，并且转码。
参数说明：
	pSrc:指向Payload数据，包含检验和，不包括帧头帧尾
	srcLen：Src数据长度。
	srcInd: 处理位置
	pDst: 目标数据Buff
	dstLen：目标Buff数据长度
*/
Bool Utp_FramePkt(const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize)
{
	int j = 0;
	int i = 0;
	int bRet = True;
	
	if(*srcInd == 0)
		pDst[j++] = UTP_FRAME_HEAD;
	
	for(i = *srcInd; i < srcLen; i++)
	{		
		if (UTP_FRAME_HEAD == pSrc[i])
		{
			pDst[j++] = UTP_FRAME_ESCAPE;
			pDst[j++] = UTP_FRAME_ESCAPE_HREAD;
		}
		else if (UTP_FRAME_TAIL == pSrc[i])
		{
			pDst[j++] = UTP_FRAME_ESCAPE;
			pDst[j++] = UTP_FRAME_ESCAPE_TAIL;
		}
		else if (UTP_FRAME_ESCAPE == pSrc[i])
		{
			pDst[j++] = UTP_FRAME_ESCAPE;
			pDst[j++] = UTP_FRAME_ESCAPE_ESCAPE;
		}
		else
		{
			pDst[j++] = pSrc[i];
		}

		//pDst buffer 满
		if(j >= (*dstSize) - 2)
		{
			++i;
			if(i == srcLen)
			{
				break;
			}
			else
			{
				bRet = False;
				goto End;
			}
		}
	}
	
	pDst[j++] = UTP_FRAME_TAIL;
	
End:	
	*srcInd = i;
	*dstSize = j;
	return bRet;
}

//每次发送一帧字节
uint16_t Utp_SendFrame(Utp* pUtp, const void* pData, uint16_t len)
{
	#define BUF_SIZE 64
	int i = 0;
	uint8_t byte[BUF_SIZE];
	int j = 0;

	while(i < len)
	{
		j = sizeof(byte);
		Utp_FramePkt((uint8_t*)pData, len, &i, byte, &j);
		pUtp->TxFn(byte, j);
	}
	
	return len;
}

Bool Utp_isBusy(Utp* pUtp)
{
	return (pUtp->m_state != UTP_FSM_INIT);
}

const UtpCmd* Utp_FindCmdItem(Utp* pUtp, uint8_t cmd)
{
	const UtpCmd* pCmd = pUtp->m_cmds;
	for (int i = 0; i < pUtp->m_cmdCount; i++, pCmd++)
	{
		if (cmd == pCmd->cmd)
		{
			return pCmd;
		}
	}
	return Null;
}

void Utp_ResetReq(Utp* pUtp)
{
	pUtp->m_state = UTP_FSM_INIT;
	pUtp->m_ReqLen = 0;
	pUtp->m_reTxCount = 0;
	SwTimer_Stop(&pUtp->m_Timer);
}

void Utp_ResetRsp(Utp* pUtp)
{
	pUtp->m_pCurrentCmd = Null;

	Queue_reset(&pUtp->m_rxQueue);
	pUtp->m_searchIndex = 0;
	pUtp->m_isFoundHead = 0;
	pUtp->m_rxDataTicks = 0;
}

void Utp_Reset(Utp* pUtp)
{
	//如果当前有正在发送的命令，立刻终止
	if(pUtp->m_pCurrentCmd)
	{
		Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
	}
	
	Utp_ResetRsp(pUtp);
	Utp_ResetReq(pUtp);
	
	const UtpCmd* pCmd = pUtp->m_cmds;
	for (int i = 0; i < pUtp->m_cmdCount; i++, pCmd++)
	{
		//停止所有待命令发送
		pCmd->pExt->sendDelayMs = 0;
		if(pCmd->type == UTP_WRITE && pCmd->pData && pCmd->pStorage)
		{
			memcpy(pCmd->pData, pCmd->pStorage, pCmd->dataLen);
		}
	}
}

static OP_CODE Utp_Event(Utp* pUtp, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	OP_CODE op = pCmd->Event ? pCmd->Event(pUtp->m_pCbObj, pCmd, ev) : OP_SUCCESS;

	//调用Utp对象的回调函数
	pUtp->TresferEvent ? pUtp->TresferEvent(pUtp->m_pCbObj, pCmd, ev) : OP_SUCCESS;

	return op;
}

Bool Utp_VerifyFrame(const UtpFrame* pFrame, int frameLen)
{
	uint8_t checkSum = 0;
	uint16_t dstCheckSum = 0;

	//verify checksum.
	dstCheckSum = (uint8_t)CheckSum_Get(&dstCheckSum, &pFrame->vendor, frameLen - 2);
	if(pFrame->checkSum != dstCheckSum)
	{
		UTP_PRINTF("Crc[0x%02x, 0x%02x] error:", checkSum, dstCheckSum);
		goto Error;
	}

	return True;
	
Error:
	DUMP_BYTE(pFrame, frameLen);
	return False;
}

static Bool Utp_RspProc(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	//ASRT(pUtp->RspProc);
	pUtp->m_state = UTP_FSM_INIT;

	pUtp->m_pCurrentCmd->pExt->rcvRspErr = rspCode;
	if(rspCode == RSP_SUCCESS)
	{
		Utp_RcvRsp(pUtp, pRsp, rspCode, frameLen);
	}
	else
	{
		pUtp->m_pCurrentCmd->pExt->transferData = Null;
		pUtp->m_pCurrentCmd->pExt->transferLen = 0;
		Utp_Event(pUtp, pUtp->m_pCurrentCmd, UTP_REQ_FAILED);
	}
	
	if(UTP_FSM_INIT == pUtp->m_state)
	{
		Utp_ResetReq(pUtp);
		Utp_ResetRsp(pUtp);
	}

	pUtp->m_rxRspTicks = GET_TICKS();
	return True;
}

static void Utp_ReqProc(Utp* pUtp, const UtpFrame* pReq, int frameLen)
{
	uint8_t stateTemp = pUtp->m_state;
	OP_CODE rc = OP_NO_RSP;
	uint16_t checkSum = 0;
	
	UtpFrame* pRsp = (UtpFrame*)pUtp->m_Rsp;
	
	pUtp->m_state = UTP_FSM_RX_REQ;	//pUtp置忙标志，防止上层应用在函数ReqProc内部发起新的请求。
	Utp_RcvReq(pUtp, pReq, frameLen);
	//rc = pUtp->ReqProc(pUtp, pReq->cmd, pReq->data, pReq->len, &pRsp->data[1], &pRsp->len);
	pUtp->m_state = stateTemp;
	
	pRsp->cmd = pReq->cmd;
	pRsp->data[0] = rc;	
	
	if(OP_NO_RSP != rc)
	{
		checkSum = 0;
		pRsp->checkSum = (uint8_t)CheckSum_Get(&checkSum, &pRsp->vendor, frameLen - 2);
		Utp_SendFrame(pUtp, pRsp, frameLen);
	}
}

//网络字节转换为
Bool Utp_ConvertToHost(uint8_t* dst, uint16_t dstSize, uint16_t* dstInd, FRAME_STATE *state, uint8_t byte)
{
	Bool bRet = False;
	int i = *dstInd;
	#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}

	
	if(UTP_FRAME_HEAD == byte)
	{
		i = 0;
		*state = FRAME_FOUND_HEAD;
		
		dst[i++] = byte;
	}
	else if(*state == FRAME_FOUND_HEAD)
	{
		if(UTP_FRAME_ESCAPE == byte)
		{
			*state = FRAME_FOUND_ESCAPE;
		}
		else if(UTP_FRAME_TAIL == byte)
		{
			dst[i++] = byte;
			*state = FRAME_DONE;
			bRet = True;
			goto End;
		}
		else
		{
			dst[i++] = byte;
		}
	}
	else if(*state == FRAME_FOUND_ESCAPE)
	{
		if (UTP_FRAME_ESCAPE_HREAD == byte)			byte = UTP_FRAME_HEAD;
		else if (UTP_FRAME_ESCAPE_TAIL == byte)		byte = UTP_FRAME_TAIL;
		else if (UTP_FRAME_ESCAPE_ESCAPE == byte)	byte = UTP_FRAME_ESCAPE;
		else
		{
			FRAME_RESET();
			goto End;
		}
		dst[i++] = byte;
		*state = FRAME_FOUND_HEAD;
	}
	
	if(i >= dstSize)
	{
		//Printf("*Buf is full*.\n");
//		ASRT(i < dstSize);
		FRAME_RESET();
	}

End:
	
	* dstInd = i;
	return bRet;
}

//接收帧处理
void Utp_RcvFrameHandler(Utp* pUtp, const UtpFrame* pFrame, int frameLen)
{
	//判断响应命令码是否匹配
	if (UTP_FSM_WAIT_RSP == pUtp->m_state && pUtp->m_Req[0] == pFrame->cmd)
	{
		Utp_RspProc(pUtp, pFrame, RSP_SUCCESS, frameLen);	//响应处理
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//请求处理
	}

	pUtp->m_FrameState = FRAME_INIT;
	return;
}

void Utp_RxData(Utp* pUtp, uint8_t* pData, int len)
{
	//如果接收数据间隔大于UTP_RX_DATA_INTERVAL_MS，则必须丢弃之前接收到的数据。
	if (pUtp->m_rxDataTicks && SwTimer_isTimerOutEx(pUtp->m_rxDataTicks, UTP_RX_DATA_INTERVAL_MS))
	{
		//Utp_ResetRsp(pUtp);
		//清除接收缓冲区
		Queue_reset(&pUtp->m_rxQueue);
		pUtp->m_searchIndex = 0;
		pUtp->m_isFoundHead = 0;
		pUtp->m_rxDataTicks = 0;
	}
	pUtp->m_rxDataTicks = GET_TICKS();

	Queue_writeBytes(&pUtp->m_rxQueue, pData, len);
}

//检测是否接收到数据帧
void Utp_CheckRxFrame(Utp* pUtp)
{
	if (!pUtp->m_isFoundHead)
	{
		//搜索头字节
		if (Queue_searchByte(&pUtp->m_rxQueue, UTP_FRAME_HEAD, &pUtp->m_searchIndex))
		{
			pUtp->m_isFoundHead = True;
		}
	}
	else
	{
		uint16_t frameLen = 0;
		Bool isOk = False;
		//搜索尾字节
		uint8_t* pRaw = Queue_searchByte(&pUtp->m_rxQueue, UTP_FRAME_TAIL, &pUtp->m_searchIndex);
		if (pRaw)
		{
			FRAME_STATE state = FRAME_INIT;

			for (int i = 0; i < pUtp->m_searchIndex; i++)
			{
				//转码
				isOk = Utp_ConvertToHost(pUtp->m_Frame, UTP_RSP_BUF_SIZE, &frameLen, &state, pRaw[i]);
				if (isOk)
				{
					break;
				}
			}
		}

		//从响应数据队列中移除
		Queue_popElements(&pUtp->m_rxQueue, pUtp->m_searchIndex);

		//帧校验
		if (isOk && Utp_VerifyFrame((UtpFrame*)pUtp->m_Frame, frameLen))
		{
			//帧处理
			Utp_RcvFrameHandler(pUtp, (UtpFrame*)pUtp->m_Frame, frameLen);
		}
	}
}

void Utp_Run(Utp* pUtp)
{	
	Utp_CheckRxFrame(pUtp);

	if(UTP_FSM_WAIT_RSP == pUtp->m_state)	//判断等待响应是否超时
	{
		if(SwTimer_isTimerOut(&pUtp->m_Timer))
		{
			if(pUtp->m_reTxCount >= pUtp->m_maxTxCount && pUtp->m_maxTxCount != UTP_ENDLESS)
			{
				Utp_RspProc(pUtp, Null, 0, RSP_TIMEOUT);
			}
			else
			{
				Utp_SendFrame(pUtp, pUtp->m_Req, pUtp->m_ReqLen);
				
				pUtp->m_reTxCount++;
				SwTimer_ReStart(&pUtp->m_Timer);
			}
		}
	}
	
	Utp_CheckReq(pUtp);
}

void Utp_Init(Utp* pUtp, UtpTxFn txFn, const UtpCmd* pCmds, int cmdCount, UtpCmdTransferCb eventFn, void* pCbObj)
{	
	memset(pUtp, 0, sizeof(Utp));
	Queue_init(&pUtp->m_rxQueue, pUtp->m_Rsp, 1, sizeof(pUtp->m_Rsp));

	pUtp->TxFn  		= txFn;
	pUtp->TresferEvent  = eventFn;

	pUtp->m_cmds = pCmds;
	pUtp->m_cmdCount = cmdCount;
	pUtp->m_pCbObj = pCbObj;

	pUtp->m_sendCmdIntervalMs = 10;

	SwTimer_Init(&pUtp->m_Timer, 0, 0);
}


