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
#include "type_a.h"

//static Fm175Drv g_fmDrv;

static FmFsmFn fm175Drv_findFsm(uint8 state);
void fm175Drv_irq_timeOut(Fm175Drv* pDrv);
void fm175Drv_irq_tx(Fm175Drv* pDrv);
#if 1

extern unsigned char g_iicPortAddr = 0;
//static DrvIo* g_pNfcNpdAIO = Null;
//#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 1)
//#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 0)
#define FM17522_NPD_HIGHT	//����
#define FM17522_NPD_LOW		//����


/*********************************************/
/*��������	    Pcd_ConfigISOType    */
/*���ܣ�	    ���ò���Э��    */
/*���������	type 0��ISO14443AЭ�飻    */
/*					 1��ISO14443BЭ�飻    */
/*����ֵ��	    OK    */
/*********************************************/
void fm175Drv_ConfigISOType(unsigned char type)
{
	if (type == 0)                     //ISO14443_A
	{
		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ// chenke, FM17522 ��bit4��RFTԤ��       
		Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 ����100%ASK��Ч        
		Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A// chenke, TX CRC��Ч, TXSPEED 106Kbit        
		Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A// chenke, RX CRC��Ч, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
	}
	if (type == 1)                     //ISO14443_B
	{
		Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ        
		Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE B        
		Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE B        
		Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 ����ON�絼        
		Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 ����OFF�絼        
		Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 ����100%ASK��Ч        
	}
}

void fm175Drv_event(Fm175Drv* pDrv, FM_EVENT evt, TRANSFER_RESULT res)
{
	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		Set_BitMask(ControlReg, 0x80);           // stop timer now
		Write_Reg(CommandReg, Idle);
		Clear_BitMask(BitFramingReg, 0x80);//�رշ���

		SwTimer_Stop(&pDrv->timer);
		pDrv->status = TRANSFER_STATUS_IDLE;
	}

	if (pDrv->cfg->Event) pDrv->cfg->Event(pDrv->obj, evt);
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
void fm175Drv_setRf(unsigned char mode)
{
    unsigned char result;
    
    result = Read_Reg(TxControlReg);
    
    if	((result&0x03) == mode)//chenke, �ж��Ƿ�TX1��TX2�Ѿ����
    {
    }
    if (mode==0)
	{
	    result=Clear_BitMask(TxControlReg,0x03); //�ر�TX1��TX2���
	}
    if (mode==1)
	{
    	result=Set_BitMask(TxControlReg,0x01); //����TX1���
	}
    if (mode==2)
	{
    	result=Set_BitMask(TxControlReg,0x02); //����TX2���
	}
    if (mode==3)
	{
    	result=Set_BitMask(TxControlReg,0x03); //��TX1��TX2���
	}
}

static void fm175Drv_switchState(Fm175Drv* pDrv, uint8 state)
{
	uint8 rc = 0;
	if (pDrv->state == state) return;

	if (state == FM_STATE_INIT)
	{
	}
	else if (state == FM_STATE_NPD_LOW)
	{
		FM17522_NPD_LOW;
		SwTimer_Start(&pDrv->delayTimer, 10, 0);
	}
	else if (state == FM_STATE_NPD_HIGH)
	{
		FM17522_NPD_HIGHT;
		SwTimer_Start(&pDrv->delayTimer, 10, 0);
	}
	else if (state == FM_STATE_WORK)
	{
		fm175Drv_setRf(pDrv->antPort);

		//��TX�������Ҫ��ʱ�ȴ������ز��ź��ȶ�
		SwTimer_Start(&pDrv->delayTimer, 1, 0);
	}
	else if (state == FM_STATE_WORK)
	{
		fm175Drv_ConfigISOType(0);
		rc = TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK);
		
		if (rc == 0)
		{
			rc = TypeA_RATS(0x20, pDrv->picc_ats);
		}

		pDrv->cardIsExist = (rc == 0);
		fm175Drv_event(pDrv, SEARCH_CARD_DONE, 0);
	}

	pDrv->state = state;
	pDrv->fsm = fm175Drv_findFsm(state);
}

void fm175Drv_switchPort(Fm175Drv* pDrv, uint8_t port)
{
	if (g_iicPortAddr == port) return;

	fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
}

void fm175Drv_fsmInit(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
}

void fm175Drv_fsmNpdLow(Fm175Drv* pDrv)
{
	if (SwTimer_isTimerOut(&pDrv->delayTimer))
	{
		fm175Drv_switchState(pDrv, FM_STATE_NPD_HIGH);
	}
}

void fm175Drv_fsmNpdLHigh(Fm175Drv* pDrv)
{
	if (SwTimer_isTimerOut(&pDrv->delayTimer))
	{
		fm175Drv_switchState(pDrv, FM_STATE_NPD_SET_ANT);
	}
}

void fm175Drv_fsmSetAnt(Fm175Drv* pDrv)
{
	if (SwTimer_isTimerOut(&pDrv->delayTimer))
	{
		fm175Drv_switchState(pDrv, FM_STATE_WORK);
	}
}

void fm175Drv_fsmWork(Fm175Drv* pDrv)
{
	if (SwTimer_isTimerOut(&pDrv->delayTimer))
	{
		fm175Drv_irq_timeOut(pDrv);
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
		{FM_STATE_INIT		 , fm175Drv_fsmInit},
		{FM_STATE_NPD_LOW	 , fm175Drv_fsmNpdLow},
		{FM_STATE_NPD_HIGH	 , fm175Drv_fsmNpdLHigh},
		{FM_STATE_NPD_SET_ANT, fm175Drv_fsmSetAnt},
		{FM_STATE_WORK		 , fm175Drv_fsmWork},
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

void fm175Drv_fsm(Fm175Drv* pDrv)
{
	pDrv->fsm(pDrv);
}

#endif

/*********************************************/
/*��������	    fm175Drv_setTimer    */
/*���ܣ�	    ���ý�����ʱ    */
/*���������	delaytime����ʱʱ�䣨��λΪ���룩    */
/*����ֵ��	    OK    */
/*********************************************/
void fm175Drv_setTimer(unsigned long delaytime)//�趨��ʱʱ�䣨ms��
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
	Set_BitMask(TModeReg, Prescaler >> 8);
	Write_Reg(TPrescalerReg, Prescaler & 0xFF);
	Write_Reg(TReloadMSBReg, TimeReload >> 8);
	Write_Reg(TReloadLSBReg, TimeReload & 0xFF);
}



/***************************************************************
�������ܣ�����������������ᴥ�������¼���ͨ�������¼��ص�����֪ͨ���������ɹ�����ʧ�ܣ��ο��¼�����FM_EVENT��
����˵����
	pDrv��ȷ���������
	cmd����������
	txBuf�� ��������ָ��
	txBufSize�� �������ݳ���
	rxBuf��		��������ָ��
	rxBufSize�� �������ݳ���
	timeOutMs�� ��ʱʱ��
	cbObj��		�¼��ص������Ķ������
����ֵ��TRUE-���������ɹ���FALSE-��������ʧ��
***************************************************************/
Bool fm175Drv_transStart(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf
	, int txBufSize
	, void* rxBuf
	, int rxBufSize
	, uint32 timeOutMs
	, void* cbObj
)
{
	TransMgr* item = &pDrv->transMgr;

	if (!fm175Drv_isIdle(pDrv)) return False;

	memset(item, 0, sizeof(TransMgr));

	pDrv->cmd = cmd;
	pDrv->obj = cbObj;

	item->totalLen = txBufSize;
	item->offset = 0;

	item->transBufLen = MIN(txBufSize, pDrv->txBufSize);
	item->transBufOffset = 0;
	item->transLen = 0;

	pDrv->txBuf = txBuf;
	pDrv->txBufSize = txBufSize;
	pDrv->rxBuf = rxBuf;
	pDrv->rxBufSize = rxBufSize;

	Clear_FIFO();

	Write_Reg(CommandReg, Idle);
	//    rx_temp = Read_Reg(CommandReg);

	Write_Reg(WaterLevelReg, pDrv->cfg->waterLevel);//����WaterLeve
//   	rx_temp = Read_Reg(WaterLevelReg);

	Write_Reg(ComIrqReg, 0x7F);//���IRQ��־
//	rx_temp = Read_Reg(ComIrqReg);

	fm175Drv_setTimer(timeOutMs);
	Set_BitMask(TModeReg, 0x80);//�Զ�������ʱ��
	Write_Reg(CommandReg, cmd);

	pDrv->status = TRANSFER_STATUS_TX;
	SwTimer_Start(&pDrv->timer, timeOutMs + 10, 0);

	fm175Drv_event(pDrv, TRANS_TX_START, TRANS_RESULT_SUCCESS);

	fm175Drv_irq_tx(pDrv);

	return True;
}

void fm175Drv_irq_rxDone(Fm175Drv* pDrv)
{
	fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
}

void fm175Drv_irq_rx(Fm175Drv* pDrv)
{
	TransMgr* item = &pDrv->transMgr;

	item->transLen = Read_Reg(FIFOLevelReg);

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

		Read_FIFO(readLen, &pDrv->rxBuf[item->transBufOffset]); //����FIFO����
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
	int fifeSize = Read_Reg(FIFOLevelReg);
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
		Write_FIFO(item->transLen, (uint8*)&pDrv->txBuf[item->transBufOffset]);
		Set_BitMask(BitFramingReg, 0x80);	//��������
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
	if (pDrv->status == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//��ȡ�жϴ�����
FmIrqFn fm175Drv_getIrqHandler(Fm175Drv* pDrv, uint8 irq)
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

void fm175Drv_irq(Fm175Drv* pDrv)
{
	uint8 irq = Read_Reg(ComIrqReg);//��ѯ�жϱ�־		

	for (int i = 0; i < 8; i++)
	{
		if (irq & BIT(i))
		{
			FmIrqFn handler = fm175Drv_getIrqHandler(pDrv, BIT(i));
			if (handler)
			{
				handler(pDrv);
			}

			Write_Reg(ComIrqReg, BIT(i));
		}
	}
}

void fm175Drv_run(Fm175Drv* pDrv)
{
	fm175Drv_fsm(pDrv);
}

void fm175Drv_start(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_INIT);
}

void fm175Drv_init(Fm175Drv* pDrv, const TransProtocolCfg* cfg, void* evtObj)
{
	memset(pDrv, 0, sizeof(Fm175Drv));

	pDrv->cfg = cfg;
	pDrv->obj = evtObj;
	pDrv->status = TRANSFER_STATUS_IDLE;
}
