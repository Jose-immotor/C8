
#include "Common.h"
#include "ArchDef.h"
#include "Modbus.h"
#include "Bit.h"

static Bool Mod_RspProc(Mod* pMod, const uint8_t* pRsp, int frameLen, MOD_RSP_RC rspCode);
static void Mod_RcvRsp(Mod* pMod, const uint8_t* pRsp, int frameLen, MOD_RSP_RC rspCode);

uint16_t Mod_CalcCrc(unsigned short crc, const uint8_t* pData, int len)
{
	uint8_t ch;
	for (int i = 0; i < len; i++, pData++)
	{
		ch = *pData;
		ch = (ch ^ (uint8_t)(crc & 0x00FF));
		ch = (ch ^ (ch << 4));

		crc = (crc >> 8) ^ ((uint16_t)ch << 8) ^ ((uint16_t)ch << 3) ^ ((uint16_t)ch >> 4);
	}

	return crc;
}

static uint16_t Mod_SendFrame(Mod * pMod, const void* pData, uint16_t len)
{
	pMod->busErr = BUS_ERR_OK;
	pMod->cfg->TxFn(pData, len);
	return len;
}

#define Mod_getCmdType(pCmd) pCmd->type

uint8_t* Mod_getRspData(const ModCmd* pCmd, const uint8_t* pRsp, int len, uint8_t* dlc)
{
	struct
	{
		uint8_t cmd;		//������
		int dlcInd;			//���ݳ�������Ӧ֡�е�λ�ã�-1��ʾ��Чֵ�����������dlcָ������
		uint8_t dataInd;	//��������Ӧ֡�е�λ��
		int dlc;			//ָ�����ݵĳ���
	}
	static const rspFmt[] = 
	{
		{MOD_READ_COIL_STATUS, 2 , 3},
		{MOD_READ_HOLDING_REG, 2 , 3},
		{MOD_WEITE_SINGLE_REG, -1, 4, 2},
	};

	if (pRsp[1] > 0x80)
	{
		*dlc = len - 2;
		return (uint8_t*)&pRsp[2];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(rspFmt); i++)
	{
		if (pRsp[1] == rspFmt[i].cmd)
		{
			*dlc = (uint8_t)((rspFmt[i].dlcInd == -1) ? rspFmt[i].dlc : pRsp[rspFmt[i].dlcInd]);
			return (uint8_t*)&pRsp[rspFmt[i].dataInd];
			break;
		}
	}

	Assert(False);
	return Null;
}

static int Mod_frameBuild(Mod* pMod, const ModCmd* pCmd, uint8_t* rspFrame)
{
	int ind = 2;
	rspFrame[0] = pMod->addr;
	rspFrame[1] = pCmd->modCmd;

	if (pCmd->pParam)
	{
		memcpy(&rspFrame[ind], pCmd->pParam, pCmd->paramLen);
		ind += pCmd->paramLen;
	}

	//���д����
	if (Mod_getCmdType(pCmd) == MOD_WRITE)
	{
		if (pCmd->pStorage)
		{
			memcpy(&rspFrame[ind], pCmd->pStorage, pCmd->storageLen);
			ind += pCmd->storageLen;
		}
	}

	uint16 crc = Mod_CalcCrc(0x6363, rspFrame, ind);

	rspFrame[ind++] = (uint8_t)(crc >> 8);
	rspFrame[ind++] = (uint8_t)(crc);

	return ind;
}

static Bool Mod_FrameVerify(Mod * pMod, const uint8_t * pData, int len, const uint8_t * pReq)
{
	if (pReq)
	{
		if (pReq[0] != pData[0]) return False;
		if (pReq[1] != pData[1] && (pReq[1] + 0x80) != pData[1]) return False;
	}
	else
	{
		if (pReq[0] != pMod->addr) return False;
	}

	uint16 calcCrc = Mod_CalcCrc(0x6363, pData, len - 2);
	uint16 crc = AS_UINT16(pData[len - 2], pData[len - 1]);
	return (calcCrc == crc);
}

/*
* ��ȡ��һ�����͵�����
* isPending��
*/
static const ModCmd* Mod_GetNextSendCmd(Mod* pMod, Bool isPending)
{
	ModCmdEx* pExt;
	const ModCmd* pCmd = pMod->cfg->cmdArray;
	for (int i = 0; i < pMod->cfg->cmdCount; i++, pCmd++)
	{
		pExt = pCmd->pExt;
		if (pExt == Null) continue;

		if (pExt->isForceSend) return pCmd;

		//�Ƿ��д�����READ/WRITE����(pExt->sendDelayMs > 0)
		if (pExt->sendDelayMs)
		{
			if (SwTimer_isTimerOutEx(pExt->rxRspTicks, pExt->sendDelayMs))
			{
				return pCmd;
			}
			else if (isPending)
			{
				return pCmd;
			}
		}

		//�Ƿ��д�����WRITE����
		if (Mod_getCmdType(pCmd) == MOD_WRITE && pCmd->pStorage && pCmd->mirror)
		{
			if (memcmp(pCmd->pStorage, pCmd->mirror, pCmd->storageLen) != 0)
			{
				return pCmd;
			}
		}
	}

	return Null;
}

const ModCmd* Mod_FindCmdItem(const ModCmd * pCmd, int count, uint8_t cmd)
{
	for (int i = 0; i < count; i++, pCmd++)
	{
		if (cmd == pCmd->cmd)
		{
			return pCmd;
		}
	}
	return Null;
}

static void Mod_ResetTxBuf(Mod * pMod)
{
	pMod->state = MOD_FSM_INIT;
	pMod->txBufLen = 0;
	pMod->reTxCount = 0;
}

static void Mod_ResetRxBuf(Mod * pMod)
{
}

void Mod_ResetCmds(const ModCfg * cfg)
{
	const ModCmd* pCmd = cfg->cmdArray;
	for (int i = 0; i < cfg->cmdCount; i++, pCmd++)
	{
		//ֹͣ���д������
		pCmd->pExt->sendDelayMs = 0;
		pCmd->pExt->isForceSend = 0;
		if (Mod_getCmdType(pCmd) == MOD_WRITE && pCmd->mirror && pCmd->pStorage)
		{
			memcpy(pCmd->mirror, pCmd->pStorage, pCmd->paramLen);
		}
	}
}

/************************************
�������ܣ�Э�����λ��
	ȡ���Ѿ��������ڵȴ���Ӧ�����
	ȡ�����еȴ����͵�����
	���Rx��������Req������
����˵����
	pMod��Mod����
����ֵ��
***************************************/
void Mod_Reset(Mod * pMod)
{
	//�����ǰ�����ڷ��͵����������ֹ
	if (pMod->pWaitRspCmd)
	{
		Mod_RspProc(pMod, Null, 0, MOD_RSP_CANCEL);
	}
	Mod_ResetRxBuf(pMod);
	Mod_ResetTxBuf(pMod);
	Mod_ResetCmds(pMod->cfg);
}

static MOD_EVENT_RC Mod_Event(Mod * pMod, const ModCmd * pCmd, MOD_TXF_EVENT ev)
{
	const ModCfg* cfg = pMod->cfg;
	MOD_EVENT_RC op = pCmd->Event ? pCmd->Event(cfg->pCbObj, pCmd, ev) : MOD_EVENT_RC_SUCCESS;
	MOD_EVENT_RC opRc = cfg->TresferEvent ? cfg->TresferEvent(cfg->pCbObj, pCmd, ev) : MOD_EVENT_RC_SUCCESS;

	//2������ֵ��ֻҪ��һ��Ϊ��MOD_EVENT_RC_SUCCESS������ʧ��ֵ
	return (op == MOD_EVENT_RC_SUCCESS) ? opRc : op;
}


static Bool Mod_RspProc(Mod * pMod, const uint8_t * pRsp, int frameLen, MOD_RSP_RC rspCode)
{
	pMod->pWaitRspCmd->pExt->rcvRspErr = rspCode;
	if (rspCode == MOD_RSP_SUCCESS)
	{
		Mod_RcvRsp(pMod, pRsp, rspCode, frameLen);
	}
	else
	{
		pMod->pWaitRspCmd->pExt->transferData = Null;
		pMod->pWaitRspCmd->pExt->transferLen = 0;
		Mod_Event(pMod, pMod->pWaitRspCmd, MOD_REQ_FAILED);
	}

	//��Ӧ��������
	SwTimer_Stop(&pMod->waitRspTimer);
	pMod->pWaitRspCmd = Null;
	pMod->rxRspTicks = GET_TICKS();

	Mod_ResetTxBuf(pMod);
	return True;
}


static void Mod_ReqProc(Mod * pMod, const uint8_t * pReq, int frameLen)
{
	//const ModFrameCfg* frameCfg = pMod->frameCfg;
	//uint8_t rc = frameCfg->result_UNSUPPORTED;
	//uint8_t* txBuf = frameCfg->txBuf;
	//const uint8_t* pData = &pReq[MODBUS_CMD_IND];
	//const ModCmd* pCmd = Mod_FindCmdItem(pMod->cfg->cmdArray, pMod->cfg->cmdCount, pReq[MODBUS_CMD_IND]);
	//int dlc = 1;

	//if (pCmd)
	//{
	//	rc = frameCfg->result_SUCCESS;
	//	frameLen -= pReq[MODBUS_LEN_IND];

	//	//��������
	//	pCmd->pExt->transferData = pData;
	//	pCmd->pExt->transferLen = frameLen;

	//	if (pCmd->pStorage && pCmd->storageLen)
	//	{
	//		if (memcmp(pCmd->pStorage, pData, pCmd->storageLen) != 0)
	//		{
	//			MOD_EVENT_RC evRc = Mod_Event(pMod, pCmd, MOD_CHANGED_BEFORE);
	//			if (evRc == MOD_EVENT_RC_SUCCESS)
	//			{
	//				memcpy(pCmd->pStorage, pData, MIN(pCmd->storageLen, frameLen));
	//				Mod_Event(pMod, pCmd, MOD_CHANGED_AFTER);
	//			}
	//		}
	//	}

	//	if (pCmd->pData)
	//	{
	//		pCmd->pExt->transferData = pCmd->pData;
	//		pCmd->pExt->transferLen = pCmd->dataLen;
	//	}
	//	rc = Mod_Event(pMod, pCmd, MOD_GET_RSP);
	//	if (rc == frameCfg->result_SUCCESS && pCmd->pExt->transferData)
	//	{
	//		memcpy(&txBuf[MODBUS_DATA_IND + 1], pCmd->pExt->transferData, pCmd->pExt->transferLen);
	//	}
	//	dlc += pCmd->pExt->transferLen;

	//	Mod_Event(pMod, pCmd, MOD_REQ_SUCCESS);
	//}

	////pCmd==Null��˵������û��ʵ�֣�����UNSUPPORTED
	//if (pCmd==Null || pCmd->type != MOD_NOTIFY)
	//{
	//	txBuf[MODBUS_CMD_IND] = pReq[MODBUS_CMD_IND];
	//	txBuf[MODBUS_DATA_IND] = rc;

	//	pMod->txBufLen = Mod_frameBuild(pMod
	//		, pCmd
	//		, &txBuf[MODBUS_DATA_IND]
	//		, dlc
	//		, pReq
	//		, txBuf);

	//	Mod_SendFrame(pMod, txBuf, frameLen);
	//}

	//Mod_ResetTxBuf(pMod);
}

//����֡����
void Mod_RcvFrameHandler(Mod * pMod, const uint8_t * pFrame, int frameLen)
{
	if (MOD_FSM_WAIT_RSP == pMod->state)
	{
		//�ж�����֡����Ӧ֡�Ƿ�ƥ��
		if (Mod_FrameVerify(pMod, pFrame, frameLen, pMod->frameCfg->txBuf))
		{
			//Mod_RspProc(pMod, pFrame, frameLen, MOD_RSP_SUCCESS);	//��Ӧ����
		}
	}
	else
	{
		Mod_ReqProc(pMod, pFrame, frameLen);	//������
	}

	return;
}


static Bool Mod_SendReq(Mod * pMod, const ModCmd * pCmd, uint32_t waitMs, uint8_t maxReTxCount)
{
	if (pMod->state != MOD_FSM_INIT)
	{
		//Printf("Mod is busy.\n");
		return False;
	}

	Mod_ResetTxBuf(pMod);
	//Mod_ResetRxBuf(pMod);

	pMod->txBufLen = Mod_frameBuild(pMod, pCmd, pMod->frameCfg->txBuf);

	pMod->reTxCount = 1;
	pMod->maxTxCount = maxReTxCount;
	Mod_SendFrame(pMod, pMod->frameCfg->txBuf, pMod->txBufLen);

	if (waitMs)
	{
		pMod->state = MOD_FSM_WAIT_RSP;
		SwTimer_Start(&pMod->waitRspTimer, waitMs, 0);
	}
	else
	{
		Mod_ResetTxBuf(pMod);
	}

	return True;
}

/*******************************************
�������ܣ��������
����˵����
	pMod��Mod����
	cmd�����͵������롣
����ֵ����
*******************************************/
void Mod_SendCmd(const ModCfg * cfg, uint8_t cmd)
{
	const ModCmd* pCmd = Mod_FindCmdItem(cfg->cmdArray, cfg->cmdCount, cmd);
	if (pCmd && pCmd->pExt)
	{
		pCmd->pExt->isForceSend = True;
	}
}

/*******************************************
�������ܣ���ʱ�������
����˵����
	pMod��Mod����
	cmd�����͵������롣
	delayMs����ʱ�����¼���0��ʾ����ʱ����������
����ֵ����
*******************************************/
void Mod_DelaySendCmd(const ModCfg * cfg, uint8_t cmd, uint32_t delayMs)
{
	const ModCmd* pCmd = Mod_FindCmdItem(cfg->cmdArray, cfg->cmdCount, cmd);
	if (delayMs == 0) delayMs = 1;	//delayMs=1����ʾ��������
	if (pCmd && pCmd->pExt)
	{
		pCmd->pExt->sendDelayMs = delayMs;
	}
}

static void Mod_RcvRsp(Mod * pMod, const uint8_t * pRsp, int frameLen, MOD_RSP_RC rspCode)
{
	const ModFrameCfg* frameCfg = pMod->frameCfg;
	const ModCmd* pCmd = pMod->pWaitRspCmd;
	uint8_t rspDlc = 0;

	if (pCmd == Null) return;

	pCmd->pExt->transferData = Mod_getRspData(pCmd, pRsp, frameLen, &rspDlc);
	pCmd->pExt->transferLen = rspDlc;
	pCmd->pExt->rxRspTicks = GET_TICKS();

	int minlen = MIN(rspDlc, pCmd->storageLen);
	if (pRsp[MODBUS_CMD_IND] == pCmd->modCmd)
	{
		if (pRsp && pCmd->pStorage && pCmd->storageLen)
		{
			const uint8_t* pRspData = pCmd->pExt->transferData;
			if (Mod_getCmdType(pCmd) == MOD_WRITE)
			{
				if (rspDlc && pCmd->mirror)
				{
					//����storageֵ
					memcpy(pCmd->mirror, pRspData, MIN(rspDlc, pCmd->paramLen));
				}
			}
			else if (pCmd->pStorage)
			{
				if (memcmp(pCmd->pStorage, pRspData, minlen) != 0)
				{
					MOD_EVENT_RC evRc = Mod_Event(pMod, pCmd, MOD_CHANGED_BEFORE);
					if (evRc == MOD_EVENT_RC_SUCCESS)
					{
						//����storageֵ
						memcpy(pCmd->pStorage, pRspData, minlen);
						Mod_Event(pMod, pCmd, MOD_CHANGED_AFTER);
					}
				}
				if (pCmd->mirror) memcpy(pCmd->mirror, pCmd->pStorage, pCmd->storageLen);
			}
		}

		Mod_Event(pMod, pCmd, MOD_REQ_SUCCESS);
	}
	else
	{
		Mod_Event(pMod, pCmd, MOD_REQ_FAILED);
	}

}

static void Mod_CheckReq(Mod * pMod)
{
	if (MOD_FSM_INIT != pMod->state) return;
	//֡����Ƿ�ʱ��
	if (!SwTimer_isTimerOutEx(pMod->rxRspTicks, pMod->frameCfg->sendCmdIntervalMs)) return;
	const ModCmd * pCmd = Mod_GetNextSendCmd(pMod, False);
	if (pCmd == Null) return;

	ModCmdEx * pExt = pCmd->pExt;

	if (Mod_getCmdType(pCmd) == MOD_NOTIFY)
	{
		Mod_SendReq(pMod, pCmd, 0, 0);
		pCmd->pExt->rxRspTicks = GET_TICKS();
		Mod_Event(pMod, pCmd, MOD_REQ_SUCCESS);
	}
	else
	{
		//����Ĭ�ϵķ��Ͳ���
		pMod->waitRspMs = pMod->frameCfg->waitRspMsDefault;	//Ĭ�ϵȴ���Ӧ��ʱ��Ϊ1��
		pMod->maxTxCount = 3;		//Ĭ�ϵ��ط�����Ϊ3
		Mod_Event(pMod, pCmd, MOD_TX_START);

		Mod_SendReq(pMod, pCmd, pMod->waitRspMs, pMod->maxTxCount);
		pMod->pWaitRspCmd = pCmd;
	}
	//������ͱ�־
	pExt->sendDelayMs = 0;
	pExt->isForceSend = False;

}

Bool Mod_isIdle(Mod * pMod)
{
	if (MOD_FSM_INIT != pMod->state) return False;
	if (Mod_GetNextSendCmd(pMod, True)) return False;

	return True;
}

Bool Mod_SwitchCfg(Mod * pMod, const ModCfg * cfg)
{
	if (MOD_FSM_INIT != pMod->state) return False;

	Mod_Reset(pMod);

	pMod->cfg = cfg;

	return True;
}

void Mod_busErr(Mod* pMod, BUS_ERR err)
{
	pMod->busErr = err;
}

/************************************
�������ܣ������߻����ж��н������ݣ����浽���ջ�����rxBufQueue������
����˵����
	pMod��Mod����
	pData�������߻����ж��н��յ������ݡ�
	len�����ݳ���,���Ϊ0����ʾ����ʧ�ܡ�
����ֵ��
***************************************/
void Mod_RxData(Mod * pMod, const uint8_t * pData, int len)
{
	pMod->busErr = BUS_ERR_OK;
	if (len)
	{
		//���������ݵļ���Ƿ�ʱ�����������붪��֮ǰ���յ������ݡ�
		pMod->rxDataTicks = GET_TICKS();

		if (pMod->rxBufLen + len <= pMod->frameCfg->rxBufLen)
		{
			memcpy(&pMod->frameCfg->rxBuf[pMod->rxBufLen], pData, len);
			pMod->rxBufLen += len;
		}
	}
}

//����Ƿ���յ�����֡
void Mod_CheckRxFrame(Mod * pMod)
{
	const ModFrameCfg* frameCfg = pMod->frameCfg;
	if (SwTimer_isTimerOutEx(pMod->rxDataTicks, pMod->frameCfg->rxIntervalMs) && pMod->rxBufLen)
	{
		pMod->rxDataTicks = 0;
		//֡У��
		if (Mod_FrameVerify(pMod, frameCfg->rxBuf, pMod->rxBufLen, Null))
		{
			//֡����
			Mod_RcvFrameHandler(pMod, frameCfg->rxBuf, pMod->rxBufLen);
		}
		pMod->rxBufLen = 0;
	}
}

void Mod_Run(Mod * pMod)
{
	Mod_CheckRxFrame(pMod);

	if (MOD_FSM_WAIT_RSP == pMod->state)	//�жϵȴ���Ӧ�Ƿ�ʱ
	{
		if (SwTimer_isTimerOut(&pMod->waitRspTimer) || pMod->busErr)
		{
			if (pMod->reTxCount >= pMod->maxTxCount && pMod->maxTxCount != MOD_ENDLESS)
			{
				if (pMod->busErr)
				{
					Mod_RspProc(pMod, Null, 0, MOD_TRANS_FAILED);
				}
				else
				{
					Mod_RspProc(pMod, Null, 0, MOD_RSP_TIMEOUT);
				}
			}
			else
			{
				Mod_SendFrame(pMod, pMod->frameCfg->txBuf, pMod->txBufLen);

				pMod->reTxCount++;
				SwTimer_ReStart(&pMod->waitRspTimer);
			}
		}
	}

	Mod_CheckReq(pMod);
}

void Mod_Init(Mod * pMod, const ModCfg * cfg, const ModFrameCfg * frameCfg)
{
	memset(pMod, 0, sizeof(Mod));

	pMod->frameCfg = frameCfg;
	pMod->cfg = cfg;

	SwTimer_Init(&pMod->waitRspTimer, 0, 0);
}

