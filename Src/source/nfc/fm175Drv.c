/*
 * File      : fm175xx.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-15     kim 		   the first version
 * ...
 */
/*************************************************************/
#include "Archdef.h"
#include "fm175Drv.h"
#include "fm175xx.h"
#include "SwTimer.h"
#include "_Macro.h"
#include "NfcIso14443.h"

//static Fm175Drv g_fmDrv;

static FmFsmFn fm175Drv_findFsm(uint8 state);
void fm175Drv_irq_timeOut(Fm175Drv* pDrv);
void fm175Drv_irq_tx(Fm175Drv* pDrv);
Bool fm175Drv_transStart(Fm175Drv* pDrv, FM17522_CMD cmd, uint32 timeOutMs);

#if 1

//static DrvIo* g_pNfcNpdAIO = Null;
//#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 1)
//#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 0)
#define FM17522_NPD_HIGHT	//����
#define FM17522_NPD_LOW		//����


void fm175Drv_event(Fm175Drv* pDrv, TRANS_EVENT evt, TRANSFER_RESULT res)
{
	pDrv->latestErr = res;
	if (pDrv->Event) pDrv->Event(pDrv->obj, evt);

	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		pDrv->txBufSize = 0;	//ȡ������

		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x80));           // stop timer now
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
		IIC_REG_ERR_RETURN(IICReg_clearBitMask(&pDrv->iicReg, BitFramingReg, 0x80));//�رշ���

		SwTimer_Stop(&pDrv->timer);
		pDrv->transStatus = TRANSFER_STATUS_IDLE;
	}
}	

/*********************************************/
/*��������	    Set_RF  */
/*���ܣ�	    ������Ƶ���    */
				
/*���������	mode����Ƶ���ģʽ  
				0���ر����
				1,����TX1���
				2,����TX2���
				3��TX1��TX2�������TX2Ϊ�������  */
/*����ֵ��	    OK
				ERROR   */
/*********************************************/
Bool fm175Drv_setRf(Fm175Drv* pDrv, unsigned char mode)
{
    unsigned char result;
    
	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(&pDrv->iicReg, TxControlReg, &result));
	if ((result & 0x03) == mode) return True;

	if (mode == 0) IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxControlReg, 0x03));	 //�ر�TX1��TX2���
	if (mode == 1) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //����TX1���
	if (mode == 2) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //����TX2���
	if (mode == 3) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //��TX1��TX2���
	
	return True;
}

void fm175Drv_irq_rxDone(Fm175Drv* pDrv)
{
	fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
}

void fm175Drv_irq_rx(Fm175Drv* pDrv)
{
	TransMgr* item = &pDrv->transMgr;

	if (!IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, (uint8*)&item->transLen)) return;

	//ѭ����ȡ����FIFO������
	for (; item->transBufOffset < item->transLen; )
	{
		int readLen = MIN(item->transBufLen - item->transBufOffset, item->transLen - item->transBufOffset);
		if (readLen == 0)
		{
			fm175Drv_event(pDrv, TRANS_RX_BUF_FULL, TRANS_RESULT_SUCCESS);
			item->transBufOffset = 0;
			continue;
		}

		IIC_REG_ERR_RETURN(IICReg_readFifo(&pDrv->iicReg, &pDrv->rxBuf[item->transBufOffset], readLen)); //����FIFO����
		item->transBufOffset += readLen;
		item->offset += readLen;
	}
	item->totalLen = item->offset;
}

void fm175Drv_irq_tx(Fm175Drv* pDrv)
{
	TransMgr* item = &pDrv->transMgr;
	const TransProtocolCfg* cfg = pDrv->cfg;
	int remainLen = 0;

	//��ǰ���ж���δ�����������FIFO
	uint8 fifeSize;
	IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &fifeSize));

	//����ʵ���Ѿ����������
	item->transLen = item->transLen - fifeSize;

	item->offset += item->transLen;
	item->transBufOffset += item->transLen;

	remainLen = item->totalLen - item->offset;
	if (remainLen == 0)
	{
		fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
		return;
	}

	if (item->transBufOffset >= item->transBufLen)
	{
		//�û�Ӧ���ڴ��¼��У�����Ҫ��������ݵ�txBuf�У���������item->transBufLen��ֵ
		fm175Drv_event(pDrv, TRANS_TX_BUF_EMPTY, TRANS_RESULT_SUCCESS);
		item->transBufOffset = 0;
	}

	//���㷢�Ͷ����ֽڵ�FIFO��
	item->transLen = MIN(item->transBufLen - item->transBufOffset, cfg->fifoDeepth - fifeSize);

	if (item->transLen)
	{
		IIC_REG_ERR_RETURN(IICReg_writeFifo(&pDrv->iicReg, & pDrv->txBuf[item->transBufOffset], item->transLen));
		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, BitFramingReg, 0x80));	//��������
	}
	else
	{
		fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
	}
}

void fm175Drv_irq_idle(Fm175Drv* pDrv)
{
}
void fm175Drv_irq_hiAlert(Fm175Drv* pDrv)
{
	fm175Drv_irq_rx(pDrv);
}
void fm175Drv_irq_loAlert(Fm175Drv* pDrv)
{
	fm175Drv_irq_tx(pDrv);
}
void fm175Drv_irq_err(Fm175Drv* pDrv)
{
	//������Mifare��Ƭ��д����ʱ�᷵��4��BIT,��Ҫ���δ�����
}
void fm175Drv_irq_timeOut(Fm175Drv* pDrv)
{
	if (pDrv->transStatus == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//��ȡ�жϴ�����
FmIrqFn fm175Drv_getIrqHandler(Fm175Drv * pDrv, uint8 irq)
{
	struct
	{
		uint8 irq;
		FmIrqFn handler;
	}
	const static g_irqHandler[] =
	{
		{REG_IRQ_MASK_TX		, (FmIrqFn)fm175Drv_irq_tx},
		{REG_IRQ_MASK_RX		, (FmIrqFn)fm175Drv_irq_rxDone},
		{REG_IRQ_MASK_IDLE		, (FmIrqFn)fm175Drv_irq_idle},
		{REG_IRQ_MASK_HI_ALERT	, (FmIrqFn)fm175Drv_irq_hiAlert},
		{REG_IRQ_MASK_LO_ALERT	, (FmIrqFn)fm175Drv_irq_loAlert},
		{REG_IRQ_MASK_ERR		, (FmIrqFn)fm175Drv_irq_err},
		{REG_IRQ_MASK_TIMEOUT	, (FmIrqFn)fm175Drv_irq_timeOut},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_irqHandler); i++)
	{
		if (irq & g_irqHandler[i].irq) return g_irqHandler[i].handler;
	}

	return Null;
}

void fm175Drv_irq(Fm175Drv * pDrv, uint8 irq)
{
	for (int i = 0; i < 8; i++)
	{
		if (irq & BIT(i))
		{
			FmIrqFn handler = fm175Drv_getIrqHandler(pDrv, BIT(i));
			if (handler)
			{
				handler(pDrv);
			}

			//���ж�
			IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, BIT(i)));
		}
	}
}

static void fm175Drv_switchState(Fm175Drv* pDrv, uint8 state)
{
//	uint8 rc = 0;
	if (pDrv->state == state) return;

	if (state == FM_STATE_INIT)
	{
	}
	else if (state == FM_STATE_NPD_LOW)
	{
		FM17522_NPD_LOW;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_NPD_HIGH)
	{
		FM17522_NPD_HIGHT;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_SEARCH_CARD)
	{
		fm175Drv_setRf(pDrv, pDrv->antPort);

		//��TX�������Ҫ��ʱ�ȴ������ز��ź��ȶ�
		SwTimer_Start(&pDrv->timer, 1, 0);
	}
	else if (state == FM_STATE_SLEEP)
	{
		//����͹���
		NfcIso14443_SoftPowerdown(pDrv);
		pDrv->antPort = 1;
	}
	else if (state == FM_STATE_ACTIVE)
	{
		SwTimer_Start(&pDrv->sleepWdTimer, 60000, 0);
		fm175Drv_event(pDrv, SEARCH_CARD_DONE, TRANS_RESULT_SUCCESS);
	}

	pDrv->state = state;
	pDrv->fsm = fm175Drv_findFsm(state);
}

void fm175Drv_fsmInit(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
	else if(msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmNpdLow(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_NPD_HIGH);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmNpdLHigh(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_SEARCH_CARD);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmSearchCard(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			NfcIso14443_ConfigISOType(pDrv, NFC_ISO_TYPE_A);
			uint8 rc = NfcIso14443_CardActivate(pDrv);

			if (rc == 0)
			{
				rc = NfcIso14443_RATS(pDrv, 0x20);
			}
			pDrv->cardIsExist = (rc == 0);

			if (pDrv->cardIsExist)
			{
				fm175Drv_switchState(pDrv, FM_STATE_ACTIVE);
			}
			else if (pDrv->antPort == 1)
			{
				pDrv->antPort = 2;
				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
			}
			else
			{
				fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_SEARCH_CARD_FAILED);
				fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
			}
		}
	}
	if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}

void fm175Drv_fsmSleep(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (pDrv->txBufSize && pDrv->transStatus == TRANSFER_STATUS_IDLE)
		{
			fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}

void fm175Drv_waitTransDone(Fm175Drv* pDrv)
{
	//ͬ�����ͷ�ʽ���������ݵȴ�����
	uint8 irq = 0;
	while (pDrv->transStatus != TRANSFER_STATUS_IDLE && !SwTimer_isTimerOut(&pDrv->timer))
	{
		if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//��ѯ�жϱ�־	
		{
			fm175Drv_irq(pDrv, irq);
		}
	}
}

void fm175Drv_fsmActive(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	//�Ƿ������ʱ��û�н��յ���Ӧ
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_irq_timeOut(pDrv);
		}
		else if (pDrv->txBufSize && pDrv->transStatus == TRANSFER_STATUS_PENDING_TX)
		{
			SwTimer_ReStart(&pDrv->sleepWdTimer);
			if (!fm175Drv_transStart(pDrv, Transceive, 200))
			{
				fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_FAILED);
			}
#if 0
			//ͬ�����ͷ�ʽ���������ݵȴ�����
			fm175Drv_waitTransDone(pDrv);
#endif
		}
		else if (SwTimer_isTimerOut(&pDrv->sleepWdTimer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		if (pDrv->transStatus == TRANSFER_STATUS_IDLE)
		{
			pDrv->iicReg.dev_addr = (uint8)param;
			fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
		}
	}
}

static FmFsmFn fm175Drv_findFsm(uint8 state)
{
	struct
	{
		uint8 state;
		FmFsmFn fsm;
	}
	static const fsmDispatch[] =
	{
		{FM_STATE_INIT			, fm175Drv_fsmInit},
		{FM_STATE_NPD_LOW		, fm175Drv_fsmNpdLow},
		{FM_STATE_NPD_HIGH		, fm175Drv_fsmNpdLHigh},
		{FM_STATE_SEARCH_CARD	, fm175Drv_fsmSearchCard},
		{FM_STATE_SLEEP			, fm175Drv_fsmSleep},
		{FM_STATE_ACTIVE		, fm175Drv_fsmActive},
	};
	for (int i = 0; i < GET_ELEMENT_COUNT(fsmDispatch); i++)
	{
		if (state == fsmDispatch[i].state)
		{
			return fsmDispatch[i].fsm;
		}
	}

	//���򲻿������е�����
	Assert(False);
	return Null;
}

void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	pDrv->fsm(pDrv, msg, param);
}

#endif

/*********************************************/
/*��������	    fm175Drv_setTimer    */
/*���ܣ�	    ���ý�����ʱ    */
/*���������	delaytime����ʱʱ�䣨��λΪ���룩    */
/*����ֵ��	    OK    */
/*********************************************/
Bool fm175Drv_setTimer(Fm175Drv* pDrv, unsigned long delaytime)//�趨��ʱʱ�䣨ms��
{
	unsigned long TimeReload;
	unsigned int Prescaler;

	Prescaler = 0;
	TimeReload = 0;
	while (Prescaler < 0xfff)
	{
		TimeReload = ((delaytime * (long)13560) - 1) / (Prescaler * 2 + 1);
		if (TimeReload < 0xffff)
			break;
		Prescaler++;
	}
	TimeReload = TimeReload & 0xFFFF;
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, Prescaler >> 8));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TPrescalerReg, Prescaler & 0xFF));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TReloadMSBReg, TimeReload >> 8));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TReloadLSBReg, TimeReload & 0xFF));
	
	return True;
}

/***************************************************************
�������ܣ������ʼ��,����Ǵ�����ݴ��䣬��Ҫ�������txBuf,�������øú����еĲ���txBuf=Null��
����˵����
	pDrv��ȷ���������
	txBuf�� ��������ָ��,���ΪNULL����ʾ�ڴ˲�ָ����������(����������)����Ҫ��TRANS_TX_BUF_EMPTY����䷢������
	txBufSize�� �������ݳ���
	rxBuf��		��������ָ��
	rxBufSize�� �������ݳ���
����ֵ��TRUE-�����ʼ���ɹ���FALSE-�����ʼ��ʧ��
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj)
{
	TransMgr* item = &pDrv->transMgr;
	//���������ڴ���
	if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	pDrv->transStatus = TRANSFER_STATUS_PENDING_TX;

	pDrv->txBuf		= txBuf;
	if (txBuf)
	{
		pDrv->txBufSize = txBufSize;
		item->transBufLen = txBufSize;
		item->totalLen = pDrv->txBufSize;
	}
	else
	{
		//���txBufΪNULL����ʾ��ָ���������ݣ���Ҫ��TRANS_TX_BUF_EMPTY����䷢������
		item->transBufLen = 0;
		item->totalLen = pDrv->txBufSize;
	}
	pDrv->rxBuf		= rxBuf;
	pDrv->rxBufSize = rxBufSize;
	pDrv->obj = pObj;

	return True;
}


/***************************************************************
�������ܣ�����������������ᴥ�������¼���ͨ�������¼��ص�����֪ͨ���������ɹ�����ʧ�ܣ��ο��¼�����FM_EVENT��
����˵����
	pDrv��ȷ���������
	cmd����������
	timeOutMs�� ��ʱʱ��
����ֵ��TRUE-���������ɹ���FALSE-��������ʧ��
***************************************************************/
Bool fm175Drv_transStart(Fm175Drv* pDrv, FM17522_CMD cmd, uint32 timeOutMs)
{
	TransMgr* item = &pDrv->transMgr;

	if (!fm175Drv_isIdle(pDrv)) return False;

	memset(item, 0, sizeof(TransMgr));

	pDrv->cmd = cmd;
	//pDrv->obj = cbObj;

	item->offset = 0;

	item->transBufOffset = 0;
	item->transLen = 0;

	//pDrv->txBuf = txBuf;
	//pDrv->txBufSize = txBufSize;
	//pDrv->rxBuf = rxBuf;
	//pDrv->rxBufSize = rxBufSize;

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearFIFO(&pDrv->iicReg));

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
	//    rx_temp = IICReg_readByte(&pDrv->iicReg, CommandReg);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->cfg->waterLevel));//����WaterLeve
//   	rx_temp = IICReg_readByte(&pDrv->iicReg, WaterLevelReg);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, 0x7F));//���IRQ��־
//	rx_temp = IICReg_readByte(&pDrv->iicReg, ComIrqReg);

	IIC_REG_ERR_RETURN_FALSE(fm175Drv_setTimer(pDrv, timeOutMs));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, 0x80));//�Զ�������ʱ��
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, cmd));

	pDrv->transStatus = TRANSFER_STATUS_TX;
	SwTimer_Start(&pDrv->timer, timeOutMs + 10, 0);

	fm175Drv_event(pDrv, TRANS_TX_START, TRANS_RESULT_SUCCESS);

	fm175Drv_irq_tx(pDrv);

	return True;
}

//ͬ����������
Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
)
{
	//����豸״̬state�Ǽ��
	if (pDrv->state == FM_STATE_ACTIVE)
	{
		//����״̬transStatusΪ��TRANSFER_STATUS_IDLE������ʧ��
		if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	}
	else if (pDrv->state == FM_STATE_SEARCH_CARD)
	{
		//����豸״̬state���ѿ������ȼ���ߣ�����״̬transStatusΪ�κ�״̬�����Է���
	}
	else //����״̬������ʧ��
	{
		return False;
	}

	int rc = False;

	//��ʱ���洫�������ͬ�����������ָ�**************************
	TransEventFn	_evtFn		= pDrv->Event	;
	FM17522_CMD		_cmd		= pDrv->cmd		;
	const void*		_txBuf		= pDrv->txBuf	;
	int				_txBufSize  = pDrv->txBufSize;
	void*			_rxBuf      = pDrv->rxBuf	;
	int				_rxBufSize  = pDrv->rxBufSize;
	/**************************************************************/

	pDrv->Event = Null;	//�¼�����ָ���ÿ�,ͬ�����䲻��ҪEvent����

	//�������䲢�ȴ����
	fm175Drv_transferInit(pDrv, txBuf, txBufSize, rxBuf, *rxBufSize, Null);
	if (fm175Drv_transStart(pDrv, cmd, timeOutMs))
	{
		fm175Drv_waitTransDone(pDrv);
	}
	else
	{
		rc = False;
		goto End; 
	}


	if (pDrv->latestErr == TRANS_RESULT_SUCCESS)
	{
		if (pDrv->rxBufSize)
		{
			*rxBufSize = pDrv->rxBufSize;
			rc = True;
			goto End;
		}
	}
End:
	//�ָ��������
	pDrv->Event		= _evtFn		;
	pDrv->cmd		= _cmd		;
	pDrv->txBuf		= _txBuf		;
	pDrv->txBufSize	= _txBufSize	;
	pDrv->rxBuf		= _rxBuf		;
	pDrv->rxBufSize	= _rxBufSize	;
	return rc;
}

Bool fm175Drv_switchNfc(Fm175Drv* pDrv, uint8 iicAddr)
{
	if (pDrv->iicReg.dev_addr == iicAddr) return True;

	fm175Drv_fsm(pDrv, FM_MSG_SWITCH_NFC, iicAddr);

	return (pDrv->iicReg.dev_addr == iicAddr);
}

void fm175Drv_run(Fm175Drv* pDrv)
{
	fm175Drv_fsm(pDrv, FM_MSG_RUN, 0);
}

void fm175Drv_start(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
}

void fm175Drv_init(Fm175Drv* pDrv, uint8 iicAddr, const TransProtocolCfg* cfg, TransEventFn	Event, void* evtObj)
{
	memset(pDrv, 0, sizeof(Fm175Drv));

	pDrv->cfg = cfg;
	pDrv->obj = evtObj;
	pDrv->transStatus = TRANSFER_STATUS_IDLE;
	pDrv->antPort = 1;
	pDrv->Event = Event;

	pDrv->iicReg.dev_addr = iicAddr;
}
