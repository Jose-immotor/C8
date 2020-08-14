
#include "ArchDef.h"
#include "Utp.h"

static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static void Utp_RcvRsp(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);

/*
把Payload数据加上帧头和帧为，并且转码。
参数说明：
	pSrc:指向Payload数据，包含检验和，不包括帧头帧尾
	srcLen：Src数据长度。
	srcInd: 处理位置
	pDst: 目标数据Buff
	dstLen：目标Buff数据长度
*/
Bool Utp_FramePkt(const UtpFrameCfg* frameCfg, const uint8_t* pSrc, uint16_t srcLen, int* srcInd, uint8_t* pDst, int* dstSize)
{
	int j = 0;
	int i = 0;
	int bRet = True;
	
	if (*srcInd == 0)
		pDst[j++] = frameCfg->head;
	
	for(i = *srcInd; i < srcLen; i++)
	{	
		if (frameCfg->head == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeHead;
		}
		else if (frameCfg->tail == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeTail;
		}
		else if (frameCfg->transcode == pSrc[i])
		{
			pDst[j++] = frameCfg->transcode;
			pDst[j++] = frameCfg->transcodeShift;
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
	
	pDst[j++] = frameCfg->tail;
	
End:	
	*srcInd = i;
	*dstSize = j;
	return bRet;
}

//网络字节转换为
Bool Utp_ConvertToHost(const UtpFrameCfg* frameCfg, uint8_t* dst, uint16_t dstSize, uint16_t* dstInd, FRAME_STATE* state, uint8_t byte)
{
	Bool bRet = False;
	int i = *dstInd;
#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}


	if (frameCfg->head == byte)
	{
		i = 0;
		*state = FRAME_FOUND_HEAD;

		dst[i++] = byte;
	}
	else if (*state == FRAME_FOUND_HEAD)
	{
		if (frameCfg->transcode == byte)
		{
			*state = FRAME_FOUND_ESCAPE;
		}
		else if (frameCfg->tail == byte)
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
	else if (*state == FRAME_FOUND_ESCAPE)
	{
		if (frameCfg->transcodeHead == byte)		byte = frameCfg->head;
		else if (frameCfg->transcodeTail == byte)	byte = frameCfg->tail;
		else if (frameCfg->transcodeShift == byte)	byte = frameCfg->transcode;
		else
		{
			FRAME_RESET();
			goto End;
		}
		dst[i++] = byte;
		*state = FRAME_FOUND_HEAD;
	}

	if (i >= dstSize)
	{
		//Printf("*Buf is full*.\n");
//		ASRT(i < dstSize);
		FRAME_RESET();
	}

End:

	*dstInd = i;
	return bRet;
}

//每次发送一帧字节
static uint16_t Utp_SendFrame(Utp* pUtp, const void* pData, uint16_t len)
{
	#define BUF_SIZE 64
	int i = 0;
	uint8_t byte[BUF_SIZE];
	int j = 0;

	while(i < len)
	{
		j = sizeof(byte);
		Utp_FramePkt(pUtp->frameCfg, (uint8_t*)pData, len, &i, byte, &j);
		pUtp->cfg->TxFn(byte, j);
	}
	
	return len;
}

static const UtpCmd* Utp_FindCmdItem(Utp* pUtp, uint8_t cmd)
{
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		if (cmd == pCmd->cmd)
		{
			return pCmd;
		}
	}
	return Null;
}

static void Utp_ResetTxBuf(Utp* pUtp)
{
	pUtp->state = UTP_FSM_INIT;
	pUtp->txBufLen = 0;
	pUtp->reTxCount = 0;
}

static void Utp_ResetRxBuf(Utp* pUtp)
{
	Queue_reset(&pUtp->rxBufQueue);
	pUtp->searchIndex = 0;
	pUtp->head = 0;
	pUtp->rxDataTicks = 0;
}

/************************************
函数功能：协议对象复位，
	取消已经发送正在等待响应的命令，
	取消所有等待发送的命令
	清除Rx缓冲区和Req缓冲区
参数说明：
	pUtp：Utp对象。
返回值无
***************************************/
void Utp_Reset(Utp* pUtp)
{
	//如果当前有正在发送的命令，立刻终止
	if(pUtp->pWaitRspCmd)
	{
		Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
	}
	
	Utp_ResetRxBuf(pUtp);
	Utp_ResetTxBuf(pUtp);
	
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		//停止所有待命令发送
		pCmd->pExt->sendDelayMs = 0;
		if(pCmd->type == UTP_WRITE && pCmd->pData && pCmd->pStorage)
		{
			memcpy(pCmd->pData, pCmd->pStorage, pCmd->dataLen);
		}
	}
}

static UTP_EVENT_RC Utp_Event(Utp* pUtp, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	const UtpCfg* cfg = pUtp->cfg;
	UTP_EVENT_RC op   = pCmd->Event ? pCmd->Event(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;
	UTP_EVENT_RC opRc = cfg->TresferEvent ? cfg->TresferEvent(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	//2个返回值，只要有一个为非UTP_EVENT_RC_SUCCESS，返回失败值
	return (op == UTP_EVENT_RC_SUCCESS) ? opRc : op;
}


static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	pUtp->pWaitRspCmd->pExt->rcvRspErr = rspCode;
	if(rspCode == RSP_SUCCESS)
	{
		Utp_RcvRsp(pUtp, pRsp, rspCode, frameLen);
	}
	else
	{
		pUtp->pWaitRspCmd->pExt->transferData = Null;
		pUtp->pWaitRspCmd->pExt->transferLen = 0;
		Utp_Event(pUtp, pUtp->pWaitRspCmd, UTP_REQ_FAILED);
	}
	
	//响应处理结束
	SwTimer_Stop(&pUtp->waitRspTimer);
	pUtp->pWaitRspCmd = Null;
	pUtp->rxRspTicks = GET_TICKS();

	Utp_ResetTxBuf(pUtp);
	return True;
}

static void Utp_ReqProc(Utp* pUtp, const uint8_t* pReq, int frameLen)
{	
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	uint8_t rc = frameCfg->result_UNSUPPORTED;
	uint8* txBuf = frameCfg->txBuf;
	const uint8* pData = &pReq[frameCfg->dataByteInd];
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, pReq[frameCfg->cmdByteInd]);
	int dlc = 1;

	if (pCmd)
	{
		rc = frameCfg->result_SUCCESS;
		frameLen -= frameCfg->dataByteInd;

		//传输数据
		pCmd->pExt->transferData = pData;
		pCmd->pExt->transferLen = frameLen;

		if (pCmd->pStorage && pCmd->storageLen)
		{
			if (memcmp(pCmd->pStorage, pData, pCmd->storageLen) != 0)
			{
				UTP_EVENT_RC evRc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
				if (evRc == UTP_EVENT_RC_SUCCESS)
				{
					memcpy(pCmd->pStorage, pData, MIN(pCmd->storageLen, frameLen));
					Utp_Event(pUtp, pCmd, UTP_CHANGED_AFTER);
				}
			}
		}

		if (pCmd->pData)
		{
			pCmd->pExt->transferData = pCmd->pData;
			pCmd->pExt->transferLen = pCmd->dataLen;
		}
		rc = Utp_Event(pUtp, pCmd, UTP_GET_RSP);
		if (rc == frameCfg->result_SUCCESS && pCmd->pExt->transferData)
		{
			memcpy(&txBuf[frameCfg->dataByteInd + 1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
		}
		dlc += pCmd->pExt->transferLen;

		Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
	}

	//pCmd==Null，说明命令没有实现，返回UNSUPPORTED
	if (pCmd==Null || pCmd->type != UTP_NOTIFY)
	{
		txBuf[frameCfg->cmdByteInd] = pReq[frameCfg->cmdByteInd];
		txBuf[frameCfg->dataByteInd] = rc;

		pUtp->txBufLen = frameCfg->FrameBuild(pUtp
			, pReq[frameCfg->cmdByteInd]
			, &txBuf[frameCfg->dataByteInd]
			, dlc
			, pReq
			, txBuf);

		Utp_SendFrame(pUtp, txBuf, frameLen);
	}

	Utp_ResetTxBuf(pUtp);
}

//接收帧处理
void Utp_RcvFrameHandler(Utp* pUtp, const uint8* pFrame, int frameLen)
{
	if (UTP_FSM_WAIT_RSP == pUtp->state)
	{
		//判断请求帧和响应帧是否匹配
		if (pUtp->frameCfg->FrameVerify(pUtp, pFrame, frameLen, pUtp->frameCfg->txBuf))
		{
			Utp_RspProc(pUtp, pFrame, frameLen, RSP_SUCCESS);	//响应处理
		}
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//请求处理
	}

	return;
}

static Bool Utp_SendReq(Utp* pUtp, uint8_t cmd, const void* pData, int len, uint32_t waitMs, uint8_t maxReTxCount)
{
	if(pUtp->state != UTP_FSM_INIT) 
	{
		//Printf("Utp is busy.\n");
		return False;
	}

	Utp_ResetTxBuf(pUtp);
	//Utp_ResetRxBuf(pUtp);

	pUtp->txBufLen = pUtp->frameCfg->FrameBuild(pUtp, cmd, pData, len, Null, pUtp->frameCfg->txBuf);

	pUtp->reTxCount = 1;
	pUtp->maxTxCount = maxReTxCount;	
	Utp_SendFrame(pUtp, pUtp->frameCfg->txBuf, pUtp->txBufLen);
	
	if(waitMs)
	{
		pUtp->state = UTP_FSM_WAIT_RSP;
		SwTimer_Start(&pUtp->waitRspTimer, waitMs, 0);
	}
	else
	{
		Utp_ResetTxBuf(pUtp);
	}

	return True;
}

/*******************************************
函数功能：发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
返回值：无
*******************************************/
void Utp_SendCmd(Utp* pUtp, uint8_t cmd)
{
	Utp_DelaySendCmd(pUtp, cmd, 0);
}

/*******************************************
函数功能：延时发送命令。
参数说明：
	pUtp：Utp对象。
	cmd：发送的命令码。
	delayMs：延时发送事件，0表示不延时，立即发送
返回值：无
*******************************************/
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs)
{
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, cmd);
	if (delayMs == 0) delayMs = 1;	//delayMs=1：表示立即发送
	if (pCmd && pCmd->pExt)
	{
		pCmd->pExt->sendDelayMs = delayMs;
	}
}

static void Utp_RcvRsp(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	uint8_t rspDlc = frameLen - frameCfg->dataByteInd;
	const UtpCmd* pCmd = pUtp->pWaitRspCmd;

	if(pCmd == Null) return;
	if(pCmd->cmd != pRsp[frameCfg->cmdByteInd]) return;

	pCmd->pExt->transferData = (uint8*)&pRsp[frameCfg->dataByteInd];
	pCmd->pExt->transferLen = rspDlc;
	pCmd->pExt->rxRspTicks = GET_TICKS();

	int minlen = MIN(rspDlc, pCmd->storageLen);
	if(pRsp[frameCfg->dataByteInd] == frameCfg->result_SUCCESS)
	{
		if(pRsp && pCmd->pStorage && pCmd->storageLen) 
		{
			const uint8_t* pRspData = &pRsp[frameCfg->dataByteInd + 1];
			if(pCmd->type == UTP_WRITE)
			{
				if(rspDlc && pCmd->pData) 
				{
					//更新storage值
					memcpy(pCmd->pData, pRspData, MIN(rspDlc, pCmd->dataLen));
				}
			}
			else if(pCmd->type == UTP_READ && pCmd->pStorage)
			{
				if(memcmp(pCmd->pStorage, pRspData, minlen) != 0)
				{
					UTP_EVENT_RC evRc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
					if(evRc == UTP_EVENT_RC_SUCCESS)
					{
						//更新storage值
						memcpy(pCmd->pStorage, pRspData, minlen);
						Utp_Event(pUtp, pCmd, UTP_CHANGED_AFTER);
					}
				}
			}
		}
		Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
	}
	else
	{
		Utp_Event(pUtp, pCmd, UTP_REQ_FAILED);
	}
	
}

static void Utp_CheckReq(Utp* pUtp)
{
	if(UTP_FSM_INIT != pUtp->state) return;

	UtpCmdEx* pExt;
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		pExt = pCmd->pExt;
		if(pExt == Null || pCmd->type == UTP_EVENT) continue;

		//帧间隔是否超时，
		if(!SwTimer_isTimerOutEx(pUtp->rxRspTicks, pUtp->frameCfg->sendCmdIntervalMs)) break;

		//设置默认的发送参数
		pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//默认等待响应的时间为1秒
		pUtp->maxTxCount = 3;		//默认的重发次数为3
		Utp_Event(pUtp, pCmd, UTP_TX_START);

		//是否有待发的READ/WRITE命令(pExt->sendDelayMs > 0)
		if(pExt->sendDelayMs && SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
		{
			if (pCmd->type == UTP_NOTIFY)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, 0, 0);
				pCmd->pExt->rxRspTicks = GET_TICKS();
			}
			else
			{
				if(pCmd->type == UTP_READ)
				{
					Utp_SendReq(pUtp, pCmd->cmd, pCmd->pData, pCmd->dataLen, pUtp->waitRspMs, pUtp->maxTxCount);
				}
				else
				{
					//WRITE
					Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, pUtp->waitRspMs, pUtp->maxTxCount);
				}
				pUtp->pWaitRspCmd = pCmd;
			}
			//清除发送标志
			pExt->sendDelayMs = 0;
			break;
		}

		//是否有待发的WRITE命令
		if(pCmd->type == UTP_WRITE && pCmd->pStorage && pCmd->pData)
		{
			if(memcmp(pCmd->pStorage, pCmd->pData, pCmd->storageLen) != 0)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, pUtp->waitRspMs, pUtp->maxTxCount);
				pExt->sendDelayMs = 0;
				pUtp->pWaitRspCmd = pCmd;
				break;
			}
		}
	}
}

/************************************
函数功能：从总线或者中断中接收数据，保存到接收缓冲区rxBufQueue队列中
参数说明：
	pUtp：Utp对象。
	pData：从总线或者中断中接收到的数据。
	len：数据长度。
返回值无
***************************************/
void Utp_RxData(Utp* pUtp, uint8_t* pData, int len)
{
	//检查接收数据的间隔是否超时，如果是则必须丢弃之前接收到的数据。
	if (pUtp->rxDataTicks && SwTimer_isTimerOutEx(pUtp->rxDataTicks, pUtp->frameCfg->rxIntervalMs))
	{
		//清除接收缓冲区
		Utp_ResetRxBuf(pUtp);
	}
	pUtp->rxDataTicks = GET_TICKS();

	Queue_writeBytes(&pUtp->rxBufQueue, pData, len);
}

//检测是否接收到数据帧
void Utp_CheckRxFrame(Utp* pUtp)
{
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	if (0 == pUtp->head)
	{
		//搜索头字节
		pUtp->searchIndex = 0;
		pUtp->head = Queue_searchByte(&pUtp->rxBufQueue, frameCfg->head, &pUtp->searchIndex);
		if (pUtp->head)
		{
			pUtp->headIndex = pUtp->searchIndex;
			pUtp->searchIndex++;
		}
		else if(pUtp->searchIndex)
		{
			//移除队列中前面无效的数据
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);
		}
	}
	else
	{
		uint16_t frameLen = 0;
		Bool isOk = False;
		//搜索尾字节
		if (Queue_searchByte(&pUtp->rxBufQueue, frameCfg->tail, &pUtp->searchIndex))
		{
			FRAME_STATE state = FRAME_INIT;

			for (int i = pUtp->headIndex; i <= pUtp->searchIndex; i++)
			{
				//转码
				isOk = Utp_ConvertToHost(frameCfg, frameCfg->transcodeBuf, frameCfg->transcodeBufLen, &frameLen, &state, pUtp->head[i]);
				if (isOk)
				{
					break;
				}
			}
			pUtp->head = 0;

			//数据队列中移除
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);

			//帧校验
			if (isOk && frameCfg->FrameVerify(pUtp, frameCfg->transcodeBuf, frameLen, Null))
			{
				//帧处理
				Utp_RcvFrameHandler(pUtp, frameCfg->transcodeBuf, frameLen);
			}
		}

		//如果队列满（说明丢失帧尾字节），清除队列所有内容
		if (Queue_isFull(&pUtp->rxBufQueue))
		{
			Utp_ResetRxBuf(pUtp);
		}
	}
}

void Utp_Run(Utp* pUtp)
{	
	Utp_CheckRxFrame(pUtp);

	if(UTP_FSM_WAIT_RSP == pUtp->state)	//判断等待响应是否超时
	{
		if(SwTimer_isTimerOut(&pUtp->waitRspTimer))
		{
			if(pUtp->reTxCount >= pUtp->maxTxCount && pUtp->maxTxCount != UTP_ENDLESS)
			{
				Utp_RspProc(pUtp, Null, 0, RSP_TIMEOUT);
			}
			else
			{
				Utp_SendFrame(pUtp, pUtp->frameCfg->txBuf, pUtp->txBufLen);
				
				pUtp->reTxCount++;
				SwTimer_ReStart(&pUtp->waitRspTimer);
			}
		}
	}
	
	Utp_CheckReq(pUtp);
}

void Utp_Init(Utp* pUtp, const UtpCfg* cfg, const UtpFrameCfg* frameCfg)
{
	memset(pUtp, 0, sizeof(Utp));

	pUtp->frameCfg = frameCfg;
	pUtp->cfg = cfg;

	Queue_init(&pUtp->rxBufQueue, pUtp->frameCfg->rxBuf, 1, pUtp->frameCfg->rxBufLen);
	SwTimer_Init(&pUtp->waitRspTimer, 0, 0);
}


