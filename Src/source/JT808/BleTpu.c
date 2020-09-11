
#include "Common.h"
#include "BleTpu.h"

static const UtpCmd* BleTpu_FindCmdItem(BleTpu* pBleTpu, uint8_t cmd)
{
	const UtpCmd* pCmd = pBleTpu->cfg->cmdArray;
	for (int i = 0; i < pBleTpu->cfg->cmdCount; i++, pCmd++)
	{
		if (cmd == pCmd->cmd)
		{
			return pCmd;
		}
	}
	return Null;
}

Bool BleTpu_verify(BleTpu* pBleTpu, const uint8_t* pReq, int frameLen)
{
	return (frameLen == pReq[1]);
}

static UTP_EVENT_RC BleTpu_Event(BleTpu* pBleTpu, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	const UtpCfg* cfg = pBleTpu->cfg;
	UTP_EVENT_RC opRc = cfg->TresferEvent ? cfg->TresferEvent(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;
	if (opRc != UTP_EVENT_RC_SUCCESS) return opRc;

	opRc = pCmd->Event ? pCmd->Event(cfg->pCbObj, pCmd, ev) : UTP_EVENT_RC_SUCCESS;

	return opRc;
}

uint8* BleTpu_ReqProc(BleTpu* pBleTpu, const uint8_t* pReq, int frameLen, uint8* rspLen)
{
	//֡У��
	if (!BleTpu_verify(pBleTpu, pReq, frameLen)) return Null;

	const BleTpuFrameCfg* frameCfg = pBleTpu->frameCfg;
	uint8_t rc = frameCfg->result_UNSUPPORTED;
	uint8* txBuf = frameCfg->txBuf;
	const uint8* pData = &pReq[frameCfg->dataByteInd];
	const UtpCmd* pCmd = BleTpu_FindCmdItem(pBleTpu, pReq[frameCfg->cmdByteInd]);
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
				UTP_EVENT_RC evRc = BleTpu_Event(pBleTpu, pCmd, UTP_CHANGED_BEFORE);
				if (evRc == UTP_EVENT_RC_SUCCESS)
				{
					memcpy(pCmd->pStorage, pData, MIN(pCmd->storageLen, frameLen));
					BleTpu_Event(pBleTpu, pCmd, UTP_CHANGED_AFTER);
				}
			}
		}

		if (pCmd->pData)
		{
			pCmd->pExt->transferData = pCmd->pData;
			pCmd->pExt->transferLen = pCmd->dataLen;
		}
		rc = BleTpu_Event(pBleTpu, pCmd, UTP_GET_RSP);
		if (rc == frameCfg->result_SUCCESS && pCmd->pExt->transferData)
		{
			memcpy(&txBuf[frameCfg->dataByteInd + 1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
			dlc += pCmd->pExt->transferLen;
		}

		BleTpu_Event(pBleTpu, pCmd, UTP_REQ_SUCCESS);
	}

	//pCmd==Null��˵������û��ʵ�֣�����UNSUPPORTED
	if (pCmd == Null || pCmd->type != UTP_NOTIFY)
	{
		txBuf[frameCfg->cmdByteInd] = pReq[frameCfg->cmdByteInd];
		txBuf[1] = dlc + 2;
		txBuf[frameCfg->dataByteInd] = rc;

		//BleTpu_SendFrame(pBleTpu, pReq[frameCfg->cmdByteInd], txBuf, frameLen);
		*rspLen = dlc + 2;
		return txBuf;
	}
	else
	{
		return Null;
	}

	//BleTpu_ResetTxBuf(pBleTpu);
}

void BleTpu_Init(BleTpu* pBleTpu, const UtpCfg* cfg, const BleTpuFrameCfg* frameCfg)
{
	memset(pBleTpu, 0, sizeof(BleTpu));

	pBleTpu->cfg = cfg;
	pBleTpu->frameCfg = frameCfg;

	Queue_init(&pBleTpu->rxBufQueue, pBleTpu->frameCfg->rxBuf, 1, pBleTpu->frameCfg->rxBufLen);
	SwTimer_Init(&pBleTpu->waitRspTimer, 0, 0);
}

