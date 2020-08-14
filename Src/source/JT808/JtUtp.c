
#include "common.h"
#include "JtUtp.h"
#include "CheckSum.h"

/*********************************
�������ܣ�֡У�飬ʵ��2�����ܣ�
	1�����pReq=Null ,����������յ��������Ƿ�������
	2�����pReq!=Null,��������յ�����֡�Ƿ��Reqƥ�䡣
����˵����
	pUtp��UTP����ָ�롣
	rxFrame��Ҫ���������֡��
	frameLen��֡���ȡ�
	req������֡�����req=Null����ʾ����Ҫ����Req
����ֵ��У������
***********************************/
static Bool Utp_VerifyFrame(Utp* pUtp, const JtUtp* pFrame, int frameLen, const JtUtp* pReq)
{
	//���pReq != Null, ��������յ�����֡�Ƿ��Reqƥ�䡣
	if (pReq)
	{
		if (pFrame->cmd != pReq->cmd)
		{
			Printf("Cmd[0x%02x, 0x%02x] not match:", pReq->cmd, pFrame->cmd);
			goto Error;
		}
	}
	else
	{
		//����֡����
		if (frameLen < pUtp->frameCfg->dataByteInd)
		{
			Printf("frame len error.(%d)\n", frameLen);
			return False;
		}

		//���pReq=Null ,����������յ��������Ƿ�������
		uint16_t dstCheckSum = 0;
		dstCheckSum = (uint8_t)CheckSum_Get(&dstCheckSum, &pFrame->vendor, frameLen - 2);
		if (pFrame->checkSum != dstCheckSum)
		{
			Printf("Crc[0x%02x, 0x%02x] error:", pFrame->checkSum, dstCheckSum);
			goto Error;
		}
	}
	return True;

Error:
	DUMP_BYTE(pFrame, frameLen);
	return False;
}

/*********************************
�������ܣ���Ҫ���͵��������ݴ����Ϊһ����������֡������Ӧ֡���ɲ���reqFrame������
	1�����pReq=Null ,���һ������֡��
	2�����pReq!=Null,���һ����Ӧ֡��
����˵����
	pUtp��UTP����ָ�롣
	cmd�������롣
	pData������ָ�롣
	len�����ݳ���
	reqFrame�����=NULL����ʾ��������֡�����Ϊ��Ӧ֡�����!=NULL����ʾ���һ������֡��
	rspFrame�����������Ҫ���������ָ֡��
����ֵ������֡���ȡ�
***********************************/
static int Utp_BuildFrame(Utp* pUtp, uint8_t cmd, const void* pData, int len, const JtUtp* pReq, JtUtp* frame)
{
	uint16_t checkSum = 0;

	Assert(pUtp->frameCfg->txBufLen > (len + pUtp->frameCfg->dataByteInd));

	if (pReq)	//���һ����Ӧ֡
	{
		memcpy(frame, pReq, pUtp->frameCfg->dataByteInd);
	}
	else
	{	//���һ������֡
		frame->ver = 0;
		frame->vendor = 0;
		frame->devType = 0;
		frame->cmd = cmd;
	}
	memcpy(frame->data, pData, len);

	frame->checkSum = (uint8_t)CheckSum_Get(&checkSum, &frame->vendor, len + 4);

	return len + pUtp->frameCfg->dataByteInd;
}

static uint8_t g_JtUtp_txBuff[128];			//�������ݻ�����
static uint8_t g_JtUtp_transcodeBuf[150];	//�������ݵ�ת�뻺����
static uint8_t g_JtUtp_rxBuff[200];			//�������ݻ�����
const UtpFrameCfg g_jtJtUtpCfg =
{
	//֡��������
	.head = 0x7E,
	.tail = 0x7E,
	.transcode = 0x7D,
	.transcodeHead = 0x02,
	.transcodeTail = 0x02,
	.transcodeShift = 0x01,
	.cmdByteInd  = 5,
	.dataByteInd = 6,

	//����Э�黺����
	.txBufLen = sizeof(g_JtUtp_txBuff),
	.txBuf	  = g_JtUtp_txBuff,
	.rxBufLen = sizeof(g_JtUtp_rxBuff),
	.rxBuf	  = g_JtUtp_rxBuff,
	.transcodeBufLen = sizeof(g_JtUtp_transcodeBuf),
	.transcodeBuf = g_JtUtp_transcodeBuf,

	//�����붨��
	.result_SUCCESS = OP_SUCCESS,
	.result_UNSUPPORTED = OP_UNSUPPORTED,

	//���ô������
	.waitRspMsDefault = 1000,
	.rxIntervalMs = 1000,
	.sendCmdIntervalMs = 10,

	//��ʼ������ָ��
	.FrameVerify = (UtpFrameVerifyFn)Utp_VerifyFrame,
	.FrameBuild  = (UtpBuildFrameFn)Utp_BuildFrame,
};
