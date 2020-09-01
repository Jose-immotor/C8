/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Utp.h"

static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static void Utp_RcvRsp(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);

Bool Utp_isIdle(const Utp* pUtp)
{
	return pUtp->state == UTP_FSM_INIT;
}

/*
��Payload���ݼ���֡ͷ��֡Ϊ������ת�롣
����˵����
	pSrc:ָ��Payload���ݣ���������ͣ�������֡ͷ֡β
	srcLen��Src���ݳ��ȡ�
	srcInd: ����λ��
	pDst: Ŀ������Buff
	dstLen��Ŀ��Buff���ݳ���
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

		//pDst buffer ��
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

//�����ֽ�ת��Ϊ
Bool Utp_ConvertToHost(const UtpFrameCfg* frameCfg, uint8_t* dst, uint16_t dstSize, uint16_t* dstInd, FRAME_STATE* state, uint8_t byte)
{
	Bool bRet = False;
	int i = *dstInd;
#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}

	if (*state == FRAME_INIT)
	{
		if (frameCfg->head == byte)
		{
			i = 0;
			*state = FRAME_FOUND_HEAD;

			dst[i++] = byte;
		}
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

//ÿ�η���һ֡�ֽ�
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
�������ܣ�Э�����λ��
	ȡ���Ѿ��������ڵȴ���Ӧ�����
	ȡ�����еȴ����͵�����
	���Rx��������Req������
����˵����
	pUtp��Utp����
����ֵ��
***************************************/
void Utp_Reset(Utp* pUtp)
{
	//�����ǰ�����ڷ��͵����������ֹ
	if(pUtp->pWaitRspCmd)
	{
		Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
	}
	
	Utp_ResetRxBuf(pUtp);
	Utp_ResetTxBuf(pUtp);
	
	const UtpCmd* pCmd = pUtp->cfg->cmdArray;
	for (int i = 0; i < pUtp->cfg->cmdCount; i++, pCmd++)
	{
		//ֹͣ���д������
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

	//2������ֵ��ֻҪ��һ��Ϊ��UTP_EVENT_RC_SUCCESS������ʧ��ֵ
	return (op == UTP_EVENT_RC_SUCCESS) ? opRc : op;
}


static Bool Utp_RspProc(Utp* pUtp, const uint8_t* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	pUtp->pWaitRspCmd->pExt->rcvRspErr = rspCode;
	if(rspCode == RSP_SUCCESS)
	{
		Utp_RcvRsp(pUtp, pRsp, frameLen, rspCode);
	}
	else
	{
		pUtp->pWaitRspCmd->pExt->transferData = Null;
		pUtp->pWaitRspCmd->pExt->transferLen = 0;
		Utp_Event(pUtp, pUtp->pWaitRspCmd, UTP_REQ_FAILED);
	}
	
	//��Ӧ�������
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

		//��������
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

	//pCmd==Null��˵������û��ʵ�֣�����UNSUPPORTED
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

//����֡����
void Utp_RcvFrameHandler(Utp* pUtp, const uint8* pFrame, int frameLen)
{
	if (UTP_FSM_WAIT_RSP == pUtp->state)
	{
		//�ж�����֡����Ӧ֡�Ƿ�ƥ��
		if (pUtp->frameCfg->FrameVerify(pUtp, pFrame, frameLen, pUtp->frameCfg->txBuf))
		{
			Utp_RspProc(pUtp, pFrame, frameLen, RSP_SUCCESS);	//��Ӧ����
		}
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//������
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
�������ܣ��������
����˵����
	pUtp��Utp����
	cmd�����͵������롣
����ֵ����
*******************************************/
void Utp_SendCmd(Utp* pUtp, uint8_t cmd)
{
	Utp_DelaySendCmd(pUtp, cmd, 0);
}

/*******************************************
�������ܣ���ʱ�������
����˵����
	pUtp��Utp����
	cmd�����͵������롣
	delayMs����ʱ�����¼���0��ʾ����ʱ����������
����ֵ����
*******************************************/
void Utp_DelaySendCmd(Utp* pUtp, uint8_t cmd, uint32_t delayMs)
{
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, cmd);
	if (delayMs == 0) delayMs = 1;	//delayMs=1����ʾ��������
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
					//����storageֵ
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
						//����storageֵ
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

		//֡����Ƿ�ʱ��
		if(!SwTimer_isTimerOutEx(pUtp->rxRspTicks, pUtp->frameCfg->sendCmdIntervalMs)) break;

		//����Ĭ�ϵķ��Ͳ���
		pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//Ĭ�ϵȴ���Ӧ��ʱ��Ϊ1��
		pUtp->maxTxCount = 3;		//Ĭ�ϵ��ط�����Ϊ3
		Utp_Event(pUtp, pCmd, UTP_TX_START);

		//�Ƿ��д�����READ/WRITE����(pExt->sendDelayMs > 0)
		if(pExt->sendDelayMs && SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
		{
			if (pCmd->type == UTP_NOTIFY)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, 0, 0);
				pCmd->pExt->rxRspTicks = GET_TICKS();
				Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
				Utp_ResetTxBuf(pUtp);
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
			//������ͱ�־
			pExt->sendDelayMs = 0;
			break;
		}

		//�Ƿ��д�����WRITE����
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
�������ܣ������߻����ж��н������ݣ����浽���ջ�����rxBufQueue������
����˵����
	pUtp��Utp����
	pData�������߻����ж��н��յ������ݡ�
	len�����ݳ��ȡ�
����ֵ��
***************************************/
void Utp_RxData(Utp* pUtp, const uint8_t* pData, int len)
{
	//���������ݵļ���Ƿ�ʱ�����������붪��֮ǰ���յ������ݡ�
	if (pUtp->rxDataTicks && SwTimer_isTimerOutEx(pUtp->rxDataTicks, pUtp->frameCfg->rxIntervalMs))
	{
		//������ջ�����
		Utp_ResetRxBuf(pUtp);
	}
	pUtp->rxDataTicks = GET_TICKS();

	Queue_writeBytes(&pUtp->rxBufQueue, pData, len);
}

//����Ƿ���յ�����֡
void Utp_CheckRxFrame(Utp* pUtp)
{
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	if (0 == pUtp->head)
	{
		//����ͷ�ֽ�
		pUtp->searchIndex = 0;
		pUtp->head = Queue_searchByte(&pUtp->rxBufQueue, frameCfg->head, &pUtp->searchIndex);
		if (pUtp->head)
		{
			pUtp->headIndex = pUtp->searchIndex;
			pUtp->searchIndex++;
		}
		else if(pUtp->searchIndex)
		{
			//�Ƴ�������ǰ����Ч������
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);
		}
	}
	else
	{
		uint16_t frameLen = 0;
		Bool isOk = False;
		//����β�ֽ�
		if (Queue_searchByte(&pUtp->rxBufQueue, frameCfg->tail, &pUtp->searchIndex))
		{
			FRAME_STATE state = FRAME_INIT;

			for (int i = pUtp->headIndex; i <= pUtp->searchIndex; i++)
			{
				//ת��,ת����������ݰ���֡ͷ��֡β
				isOk = Utp_ConvertToHost(frameCfg, frameCfg->transcodeBuf, frameCfg->transcodeBufLen, &frameLen, &state, pUtp->head[i]);
				if (isOk)
				{
					break;
				}
			}
			pUtp->head = 0;

			//���ݶ������Ƴ�
			Queue_popElements(&pUtp->rxBufQueue, pUtp->searchIndex + 1);

			//֡У�飬ȥ��֡ͷ��֡β
			if (isOk && frameCfg->FrameVerify(pUtp, &frameCfg->transcodeBuf[1], frameLen-2, Null))
			{
				//֡����,ȥ��֡ͷ��֡β
				Utp_RcvFrameHandler(pUtp, &frameCfg->transcodeBuf[1], frameLen-2);
			}
		}

		//�����������˵����ʧ֡β�ֽڣ������������������
		if (Queue_isFull(&pUtp->rxBufQueue))
		{
			Utp_ResetRxBuf(pUtp);
		}
	}
}

void Utp_Run(Utp* pUtp)
{	
	Utp_CheckRxFrame(pUtp);

	if(UTP_FSM_WAIT_RSP == pUtp->state)	//�жϵȴ���Ӧ�Ƿ�ʱ
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


