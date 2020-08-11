
#include "common.h"
#include "CheckSum.h"
#include "Utp.h"

#define UTP_PRINTF Printf

static Bool Utp_RspProc(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static void Utp_RcvRsp(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode);
static OP_CODE Utp_RcvReq(Utp* pUtp, const UtpFrame* pReq, int frameLen);

/*
��Payload���ݼ���֡ͷ��֡Ϊ������ת�롣
����˵����
	pSrc:ָ��Payload���ݣ���������ͣ�������֡ͷ֡β
	srcLen��Src���ݳ��ȡ�
	srcInd: ����λ��
	pDst: Ŀ������Buff
	dstLen��Ŀ��Buff���ݳ���
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
	
	pDst[j++] = UTP_FRAME_TAIL;
	
End:	
	*srcInd = i;
	*dstSize = j;
	return bRet;
}

//�����ֽ�ת��Ϊ
Bool Utp_ConvertToHost(uint8_t* dst, uint16_t dstSize, uint16_t* dstInd, FRAME_STATE* state, uint8_t byte)
{
	Bool bRet = False;
	int i = *dstInd;
#define FRAME_RESET() {i = 0; *state = FRAME_INIT;}


	if (UTP_FRAME_HEAD == byte)
	{
		i = 0;
		*state = FRAME_FOUND_HEAD;

		dst[i++] = byte;
	}
	else if (*state == FRAME_FOUND_HEAD)
	{
		if (UTP_FRAME_ESCAPE == byte)
		{
			*state = FRAME_FOUND_ESCAPE;
		}
		else if (UTP_FRAME_TAIL == byte)
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
	pUtp->m_txBufLen = 0;
	pUtp->m_reTxCount = 0;
	SwTimer_Stop(&pUtp->m_waitRspTimer);
}

void Utp_ResetRsp(Utp* pUtp)
{
	pUtp->m_pCurrentCmd = Null;

	Queue_reset(&pUtp->m_rxBufQueue);
	pUtp->m_searchIndex = 0;
	pUtp->m_isFoundHead = 0;
	pUtp->m_rxDataTicks = 0;
}

void Utp_Reset(Utp* pUtp)
{
	//�����ǰ�����ڷ��͵����������ֹ
	if(pUtp->m_pCurrentCmd)
	{
		Utp_RspProc(pUtp, Null, 0, RSP_CANCEL);
	}
	
	Utp_ResetRsp(pUtp);
	Utp_ResetReq(pUtp);
	
	const UtpCmd* pCmd = pUtp->m_cmds;
	for (int i = 0; i < pUtp->m_cmdCount; i++, pCmd++)
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
	UTP_EVENT_RC op = pCmd->Event ? pCmd->Event(pUtp->m_pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	//����Utp����Ļص�����
	if(pUtp->TresferEvent)
		pUtp->TresferEvent(pUtp->m_pCbObj, pCmd, ev);
	//pUtp->TresferEvent ? pUtp->TresferEvent(pUtp->m_pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	return op;
}

Bool Utp_VerifyFrame(const UtpFrame* pFrame, int frameLen)
{
	uint16_t dstCheckSum = 0;

	//verify checksum.
	dstCheckSum = (uint8_t)CheckSum_Get(&dstCheckSum, &pFrame->vendor, frameLen - 2);
	if(pFrame->checkSum != dstCheckSum)
	{
		UTP_PRINTF("Crc[0x%02x, 0x%02x] error:", pFrame->checkSum , dstCheckSum);
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
	
	UtpFrame* pRsp = (UtpFrame*)pUtp->m_rxBuf;
	
	pUtp->m_state = UTP_FSM_RX_REQ;	//pUtp��æ��־����ֹ�ϲ�Ӧ���ں���ReqProc�ڲ������µ�����
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

//����֡����
void Utp_RcvFrameHandler(Utp* pUtp, const UtpFrame* pFrame, int frameLen)
{
	//�ж���Ӧ�������Ƿ�ƥ��
	if (UTP_FSM_WAIT_RSP == pUtp->m_state && pUtp->m_txBuf[0] == pFrame->cmd)
	{
		Utp_RspProc(pUtp, pFrame, RSP_SUCCESS, frameLen);	//��Ӧ����
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//������
	}

	pUtp->m_FrameState = FRAME_INIT;
	return;
}

Bool Utp_SendReq(Utp* pUtp, uint8_t cmd, const void* pData, int len, uint32_t waitMs, uint8_t maxReTxCount)
{
	uint16_t checkSum = 0;
	UtpFrame* pReq = (UtpFrame*)pUtp->m_txBuf;

	if(pUtp->m_state != UTP_FSM_INIT) 
	{
		//Printf("Utp is busy.\n");
		return False;
	}
	if(len > UTP_REQ_BUF_SIZE - 2)
	{
		return False;
	}

	Utp_ResetReq(pUtp);
	Utp_ResetRsp(pUtp);

	pReq->cmd = cmd;
	memcpy(pReq->data, pData, len);
	
	pReq->checkSum = (uint8_t)CheckSum_Get(&checkSum, &pReq->vendor, len + 4);

	pUtp->m_reTxCount = 1;
	pUtp->m_maxTxCount = maxReTxCount;
	pUtp->m_txBufLen 	   = len + UTP_MIN_LEN;
	
	Utp_SendFrame(pUtp, pReq, pUtp->m_txBufLen);
	
	if(waitMs)
	{
		pUtp->m_state = UTP_FSM_WAIT_RSP;
		SwTimer_Start(&pUtp->m_waitRspTimer, waitMs, 0);
	}
	else
	{
		Utp_ResetReq(pUtp);
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

static OP_CODE Utp_RcvReq(Utp* pUtp, const UtpFrame* pReq, int frameLen)
{
	OP_CODE rc = OP_UNSUPPORTED;
	UtpFrame* pRsp = (UtpFrame*)pUtp->m_rxBuf;
	
	const UtpCmd* pCmd = pUtp->m_cmds;
	frameLen -= UTP_HEAD_LEN;

	for (int i = 0; i < pUtp->m_cmdCount; i++, pCmd++)
	{
		if (pCmd->cmd == pReq->cmd)
		{
			//int tranlen = MIN(frameLen, pCmd->dataLen);
			rc = OP_SUCCESS;

			//��������
			pCmd->pExt->transferData = (uint8_t*)pReq->data;
			pCmd->pExt->transferLen = frameLen;

			if(pCmd->pStorage && pCmd->storageLen) 
			{
				if(memcmp(pCmd->pStorage, pReq->data, pCmd->storageLen) != 0)
				{
					rc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
					if(rc == OP_SUCCESS)
					{
						memcpy(pCmd->pStorage, pReq->data, MIN(pCmd->storageLen, frameLen));
						Utp_Event(pUtp, pCmd, UTP_CHANGED_AFTER);
					}
				}
			}

			if(pCmd->pData)
			{
				pCmd->pExt->transferData = pCmd->pData;
				pCmd->pExt->transferLen = pCmd->dataLen;
			}
			rc = Utp_Event(pUtp, pCmd, UTP_GET_RSP);
			if(rc == OP_SUCCESS && pCmd->pExt->transferData)
			{
				memcpy(&pRsp->data[1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
			}
			
			Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
			
			break;
		}
	}

	return rc;
}

static void Utp_RcvRsp(Utp* pUtp, const UtpFrame* pRsp, int frameLen, UTP_RCV_RSP_RC rspCode)
{
	OP_CODE rc = OP_UNSUPPORTED;
	const uint8_t* pRspData = &pRsp->data[1];
	uint8_t rspDlc = frameLen - UTP_HEAD_LEN;
	const UtpCmd* pCmd = pUtp->m_pCurrentCmd;

	if(pCmd == Null) return;
	if(pCmd->cmd != pRsp->cmd) return;
	pUtp->m_pCurrentCmd = Null;

	pCmd->pExt->transferData = (uint8_t*)pRsp->data;
	pCmd->pExt->transferLen = rspDlc;
	pCmd->pExt->rxRspTicks = GET_TICKS();

	int minlen = MIN(rspDlc, pCmd->storageLen);
	if(pRsp->data[0] == OP_SUCCESS)
	{
		if(pRsp && pCmd->pStorage && pCmd->storageLen) 
		{
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
					rc = Utp_Event(pUtp, pCmd, UTP_CHANGED_BEFORE);
					if(rc == OP_SUCCESS)
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
	if(UTP_FSM_INIT != pUtp->m_state) return;

	UtpCmdEx* pExt;
	const UtpCmd* pCmd = pUtp->m_cmds;
	for (int i = 0; i < pUtp->m_cmdCount; i++, pCmd++)
	{
		pExt = pCmd->pExt;
		if(pExt == Null || pCmd->type == UTP_EVENT) continue;

		//���������֡����Ƿ�ʱ��
		if(!SwTimer_isTimerOutEx(pUtp->m_rxRspTicks, pUtp->m_sendCmdIntervalMs)) break;

		//����Ĭ�ϵķ��Ͳ���
		pUtp->m_waitRspMs = 1000;	//Ĭ�ϵȴ���Ӧ��ʱ��Ϊ1��
		pUtp->m_maxTxCount = 3;		//Ĭ�ϵ��ط�����Ϊ3
		Utp_Event(pUtp, pCmd, UTP_TX_START);

		//�Ƿ��д�����READ/WRITE����(pExt->sendDelayMs > 0)
		if(pExt->sendDelayMs && SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
		{
			if(pCmd->type == UTP_READ)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pData, pCmd->dataLen, pUtp->m_waitRspMs, pUtp->m_maxTxCount);
			}
			else if (pCmd->type == UTP_NOTIFY)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pData, pCmd->dataLen, 0, 0);
				pCmd->pExt->rxRspTicks = GET_TICKS();
			}
			else
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, pUtp->m_waitRspMs, pUtp->m_maxTxCount);
			}
			pExt->sendDelayMs = 0;
			pUtp->m_pCurrentCmd = pCmd;
			break;
		}

		//�Ƿ��д�����WRITE����
		if(pCmd->type == UTP_WRITE && pCmd->pStorage && pCmd->pData)
		{
			if(memcmp(pCmd->pStorage, pCmd->pData, pCmd->storageLen) != 0)
			{
				Utp_SendReq(pUtp, pCmd->cmd, pCmd->pStorage, pCmd->storageLen, pUtp->m_waitRspMs, pUtp->m_maxTxCount);
				pExt->sendDelayMs = 0;
				pUtp->m_pCurrentCmd = pCmd;
				break;
			}
		}
	}
}

void Utp_RxData(Utp* pUtp, uint8_t* pData, int len)
{
	//����������ݼ������UTP_RX_DATA_INTERVAL_MS������붪��֮ǰ���յ������ݡ�
	if (pUtp->m_rxDataTicks && SwTimer_isTimerOutEx(pUtp->m_rxDataTicks, UTP_RX_DATA_INTERVAL_MS))
	{
		//Utp_ResetRsp(pUtp);
		//������ջ�����
		Queue_reset(&pUtp->m_rxBufQueue);
		pUtp->m_searchIndex = 0;
		pUtp->m_isFoundHead = 0;
		pUtp->m_rxDataTicks = 0;
	}
	pUtp->m_rxDataTicks = GET_TICKS();

	Queue_writeBytes(&pUtp->m_rxBufQueue, pData, len);
}

//����Ƿ���յ�����֡
void Utp_CheckRxFrame(Utp* pUtp)
{
	if (!pUtp->m_isFoundHead)
	{
		//����ͷ�ֽ�
		if (Queue_searchByte(&pUtp->m_rxBufQueue, UTP_FRAME_HEAD, &pUtp->m_searchIndex))
		{
			pUtp->m_isFoundHead = True;
		}
	}
	else
	{
		uint16_t frameLen = 0;
		Bool isOk = False;
		//����β�ֽ�
		uint8_t* pRaw = Queue_searchByte(&pUtp->m_rxBufQueue, UTP_FRAME_TAIL, &pUtp->m_searchIndex);
		if (pRaw)
		{
			FRAME_STATE state = FRAME_INIT;

			for (int i = 0; i < pUtp->m_searchIndex; i++)
			{
				//ת��
				isOk = Utp_ConvertToHost(pUtp->m_Frame, UTP_RSP_BUF_SIZE, &frameLen, &state, pRaw[i]);
				if (isOk)
				{
					break;
				}
			}
		}

		//����Ӧ���ݶ������Ƴ�
		Queue_popElements(&pUtp->m_rxBufQueue, pUtp->m_searchIndex);

		//֡У��
		if (isOk && Utp_VerifyFrame((UtpFrame*)pUtp->m_Frame, frameLen))
		{
			//֡����
			Utp_RcvFrameHandler(pUtp, (UtpFrame*)pUtp->m_Frame, frameLen);
		}
	}
}

void Utp_Run(Utp* pUtp)
{	
	Utp_CheckRxFrame(pUtp);

	if(UTP_FSM_WAIT_RSP == pUtp->m_state)	//�жϵȴ���Ӧ�Ƿ�ʱ
	{
		if(SwTimer_isTimerOut(&pUtp->m_waitRspTimer))
		{
			if(pUtp->m_reTxCount >= pUtp->m_maxTxCount && pUtp->m_maxTxCount != UTP_ENDLESS)
			{
				Utp_RspProc(pUtp, Null, 0, RSP_TIMEOUT);
			}
			else
			{
				Utp_SendFrame(pUtp, pUtp->m_txBuf, pUtp->m_txBufLen);
				
				pUtp->m_reTxCount++;
				SwTimer_ReStart(&pUtp->m_waitRspTimer);
			}
		}
	}
	
	Utp_CheckReq(pUtp);
}

void Utp_Init(Utp* pUtp, UtpTxFn txFn, const UtpCmd* pCmds, int cmdCount, UtpCmdTransferCb eventFn, void* pCbObj)
{	
	memset(pUtp, 0, sizeof(Utp));
	Queue_init(&pUtp->m_rxBufQueue, pUtp->m_rxBuf, 1, sizeof(pUtp->m_rxBuf));

	pUtp->TxFn  		= txFn;
	pUtp->TresferEvent  = eventFn;

	pUtp->m_cmds = pCmds;
	pUtp->m_cmdCount = cmdCount;
	pUtp->m_pCbObj = pCbObj;

	pUtp->m_sendCmdIntervalMs = 10;

	SwTimer_Init(&pUtp->m_waitRspTimer, 0, 0);
}


