/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 * UTP(Uart transfer protocol)ʵ���ļ�
 */

#include "ArchDef.h"
#include "Utp.h"
#include "Cirbuffer.h"
#include "debug.h"


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
static uint16_t Utp_SendFrame(Utp* pUtp, uint8_t cmd, const void* pData, uint16_t len)
{
	#define BUF_SIZE 64
	int i = 0;
	uint8_t byte[BUF_SIZE];
	int j = 0;
	if( !pUtp || !pData || !len ) return 0;

	while(i < len)
	{
		j = sizeof(byte);
		Utp_FramePkt(pUtp->frameCfg, (uint8_t*)pData, len, &i, byte, &j);
		pUtp->cfg->TxFn(cmd, byte, j);
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
	//Queue_reset(&pUtp->rxBufQueue);
	CirBuffReset(&pUtp->rxBuffCirBuff);
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
	UTP_EVENT_RC opRc = cfg->TresferEvent ? cfg->TresferEvent(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;
	if (opRc != UTP_EVENT_RC_SUCCESS) return opRc;
	
	opRc = pCmd->Event ? pCmd->Event(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	return opRc;
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
	uint8* txBuf = frameCfg->txRspBuf ? frameCfg->txRspBuf : frameCfg->txBuf;
	int txBufLen = frameCfg->txRspBuf ? frameCfg->txRspBufLen : frameCfg->txBufLen;
	const uint8* pData = &pReq[frameCfg->dataByteInd];
	const UtpCmd* pCmd = Utp_FindCmdItem(pUtp, pReq[frameCfg->cmdByteInd]);
	int dlc = 1;

	PFL(DL_JT808,"CAN REQ:%s\n",pCmd->cmdName );

	if( pCmd && (pCmd->type == UTP_EVENT || pCmd->type == UTP_EVENT_NOTIFY ))
	{
		rc = frameCfg->result_SUCCESS;
		frameLen -= frameCfg->dataByteInd;

		//��������
		pCmd->pExt->transferData = (uint8*)pData;
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
		//Ԥ��Ĭ�ϵ�Ӧ������ָ��
		pCmd->pExt->transferData = pCmd->pData;
		pCmd->pExt->transferLen = 0;//pCmd->dataLen;
		rc = Utp_Event(pUtp, pCmd, UTP_GET_RSP);
		if (rc == frameCfg->result_SUCCESS && pCmd->pExt->transferData)
		{
			if (dlc + pCmd->pExt->transferLen <= txBufLen)
			{
				memcpy(&txBuf[frameCfg->dataByteInd + 1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
				dlc += pCmd->pExt->transferLen;
			}
			else
			{
				//�����Buff���Ȳ���
				Printf("%s size not enough.", frameCfg->txRspBuf ? "txRspBuf" : "txBuf");
				Assert(False);
			}
		}

		Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
	}
	
	if (pCmd==Null || pCmd->type == UTP_EVENT )
	{
		int txlen = 0 ;
		txBuf[frameCfg->cmdByteInd] = pReq[frameCfg->cmdByteInd];
		txBuf[frameCfg->dataByteInd] = rc;

		txlen = frameCfg->FrameBuild(pUtp
			, pReq[frameCfg->cmdByteInd]
			, &txBuf[frameCfg->dataByteInd]
			, dlc
			, Null//pReq
			, txBuf);
		Utp_SendFrame(pUtp, pReq[frameCfg->cmdByteInd], txBuf, txlen );
	}
	//
	//if( pUtp->state != UTP_FSM_WAIT_RSP )
	//{
	//	Utp_ResetTxBuf(pUtp);
	//}
}



//����֡����
void Utp_RcvFrameHandler(Utp* pUtp, const uint8* pFrame, int frameLen)
{
	if (UTP_FSM_WAIT_RSP == pUtp->state)
	{
		if (pFrame[pUtp->frameCfg->cmdByteInd] == pUtp->frameCfg->txBuf[pUtp->frameCfg->cmdByteInd])
		{
			//�ж�����֡����Ӧ֡�Ƿ�ƥ��
			if (pUtp->frameCfg->FrameVerify(pUtp, pFrame, frameLen, pUtp->frameCfg->txBuf))
			{
				Utp_RspProc(pUtp, pFrame, frameLen, RSP_SUCCESS);	//��Ӧ����
			}
		}
		else if(pUtp->frameCfg->txRspBuf)
		{
			Utp_ReqProc(pUtp, pFrame, frameLen);	//������
		}
	}
	else
	{
		Utp_ReqProc(pUtp, pFrame, frameLen);	//������
	}

	return;
}

static Bool Utp_SendReq(Utp* pUtp, const UtpCmd* pCmd)
{
	if(pUtp->state != UTP_FSM_INIT) 
	{
		//Printf("Utp is busy.\n");
		return False;
	}
	//����Ĭ�ϵķ��Ͳ���
	pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//Ĭ�ϵȴ���Ӧ��ʱ��Ϊ1��
	pUtp->maxTxCount = 3;		//Ĭ�ϵ��ط�����Ϊ3
	if (UTP_EVENT_RC_SUCCESS != Utp_Event(pUtp, pCmd, UTP_TX_START)) return False;

	Utp_ResetTxBuf(pUtp);
	//Utp_ResetRxBuf(pUtp);

	pUtp->txBufLen = pUtp->frameCfg->FrameBuild(pUtp, pCmd->cmd, pCmd->pExt->transferData, pCmd->pExt->transferLen, Null, pUtp->frameCfg->txBuf);

	pUtp->reTxCount = 1;
	Utp_SendFrame(pUtp, pCmd->cmd, pUtp->frameCfg->txBuf, pUtp->txBufLen);
	
	if(pUtp->waitRspMs)
	{
		pUtp->state = UTP_FSM_WAIT_RSP;
		SwTimer_Start(&pUtp->waitRspTimer, pUtp->waitRspMs, 0);
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

	PFL(DL_JT808,"CAN RSP:%s\n",pCmd->cmdName );

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
		
		//�Ƿ��д�����READ/WRITE����(pExt->sendDelayMs > 0)
		if(pExt->sendDelayMs && SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
		{
			if (pCmd->type == UTP_READ)
			{
				pExt->transferData = pCmd->pData;
				pExt->transferLen  = pCmd->dataLen;
			}
			else
			{
				pExt->transferData = pCmd->pStorage;
				pExt->transferLen  = pCmd->storageLen;
			}

			Utp_SendReq(pUtp, pCmd);
			//������ͱ�־
			pExt->sendDelayMs = 0;

			if (pCmd->type == UTP_NOTIFY)
			{
				pCmd->pExt->rxRspTicks = GET_TICKS();
				Utp_ResetTxBuf(pUtp);
				Utp_Event(pUtp, pCmd, UTP_REQ_SUCCESS);
			}
			else
			{
				pUtp->pWaitRspCmd = pCmd;
			}
			break;
		}

		//�Ƿ��д�����WRITE����
		if(pCmd->type == UTP_WRITE && pCmd->pStorage && pCmd->pData)
		{
			if(memcmp(pCmd->pStorage, pCmd->pData, pCmd->storageLen) != 0)
			{
				pExt->transferData = pCmd->pStorage;
				pExt->transferLen  = pCmd->storageLen;

				Utp_SendReq(pUtp, pCmd);

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
#if 0
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
#else

// ��ת��
static uint16_t _ParapBuff( uint8_t* pInbuff,uint8_t*poutbuff,uint16_t len)
{
	uint16_t i = 0 , j = 0 ;
	if( !pInbuff || !pInbuff || !len ) return 0;
	//
	while( j < len )
	{
		if( pInbuff[j] == 0x7D )
		{
			if( j + 1 < len )
			{
				if( pInbuff[j+1] == 0x01 )
				{
					poutbuff[i++] = 0x7d ;
				}
				else if( pInbuff[j+1] == 0x02 )
				{
					poutbuff[i++]= 0x7E ;
				}
				else return 0;
			}
			else return 0;
			j += 2 ;
		}
        else
        {
            poutbuff[i++] = pInbuff[j++];
        }
	}
	return i ;
}



// ���ͷ β�İ�
static uint16_t _DCodeCirBuff( const UtpFrameCfg *pframeCfg, 
	pCirBuff pcirbuff, uint8_t *poutbuff , uint16_t size )
{
	uint16_t pos = 0 , len = 0 ;
	if( !pcirbuff || !poutbuff || !size || !pframeCfg ) return 0;
	//�ҵ�ͷβ�Ĵ���

UTP_REDCODE:
	while( pcirbuff->miHead != pcirbuff->miTail )
	{
		if( pcirbuff->mpBuff[pcirbuff->miHead] == pframeCfg->head ) break ;
		_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
	}
	pos = pcirbuff->miHead ;
	len = 0 ;
	while( pos != pcirbuff->miTail && len < size )
	{
		poutbuff[len++] = pcirbuff->mpBuff[pos];
		_CIR_LOOP_ADD(pos, 1, pcirbuff->miSize );
		
		if( poutbuff[len-1] == pframeCfg->tail )
		{
			if( len > 8 )
			{
				pcirbuff->miHead = pos ;
				return len ;
			}
			else if( len > 2 )	// �����
			{
				_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
				goto UTP_REDCODE;
			}
		}
	}
	// �����
	if( len >= size )	
	{
		_CIR_LOOP_ADD(pcirbuff->miHead, 1, pcirbuff->miSize );
		goto UTP_REDCODE;
	}
	return 0 ;
}



void Utp_CheckRxFrame(Utp* pUtp)
{
	uint16_t frameLen = 0;
	if( !pUtp ) return ;
	const UtpFrameCfg* frameCfg = pUtp->frameCfg;
	//
	//Printf("[%d-%d]\r\n",
	//pUtp->rxBuffCirBuff.miHead,pUtp->rxBuffCirBuff.miTail);
	while( frameLen = _DCodeCirBuff( frameCfg , &pUtp->rxBuffCirBuff ,frameCfg->transcodeBuf , frameCfg->transcodeBufLen ))
	{
		if( frameLen = _ParapBuff( frameCfg->transcodeBuf ,frameCfg->transcodeBuf, frameLen ) )
		{
			//֡����,ȥ��֡ͷ��֡β
			Utp_RcvFrameHandler(pUtp, &frameCfg->transcodeBuf[1], frameLen-2);
		}
	}
}


#endif //

void Utp_Run(Utp* pUtp)
{
	Utp_CheckRxFrame(pUtp);
	if(UTP_FSM_WAIT_RSP == pUtp->state)	//�жϵȴ���Ӧ�Ƿ�ʱ
	{
		if(SwTimer_isTimerOut(&pUtp->waitRspTimer))
		{
			if(pUtp->reTxCount >= pUtp->maxTxCount && pUtp->maxTxCount != UTP_ENDLESS)
			{
				PFL(DL_JT808,"CAN Send Cmd:0x%02X Timeout\n", pUtp->pWaitRspCmd->cmd);
				Utp_RspProc(pUtp, Null, 0, RSP_TIMEOUT);
			}
			else
			{
				PFL(DL_JT808,"CAN Resend Cmd:0x%02X\n", pUtp->pWaitRspCmd->cmd);
				Utp_SendFrame(pUtp, pUtp->pWaitRspCmd->cmd, pUtp->frameCfg->txBuf, pUtp->txBufLen);
				
				pUtp->reTxCount++;
				SwTimer_ReStart(&pUtp->waitRspTimer);
			}
		}
	}
	else
	{
		Utp_CheckReq(pUtp);
	}
}

void Utp_Init(Utp* pUtp, const UtpCfg* cfg, const UtpFrameCfg* frameCfg)
{
	memset(pUtp, 0, sizeof(Utp));

	pUtp->frameCfg = frameCfg;
	pUtp->cfg = cfg;

	//Queue_init(&pUtp->rxBufQueue, pUtp->frameCfg->rxBuf, 1, pUtp->frameCfg->rxBufLen);
	CirBuffInit(&pUtp->rxBuffCirBuff, pUtp->frameCfg->rxBuf, pUtp->frameCfg->rxBufLen);
	SwTimer_Init(&pUtp->waitRspTimer, 0, 0);
}


