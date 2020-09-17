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
#define FM17522_NPD_HIGHT	//待定
#define FM17522_NPD_LOW		//待定


void fm175Drv_event(Fm175Drv* pDrv, TRANS_EVENT evt, TRANSFER_RESULT res)
{
	pDrv->latestErr = res;
	if (pDrv->Event) pDrv->Event(pDrv->obj, evt);

	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		pDrv->txBufSize = 0;	//取消传输

		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x80));           // stop timer now
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
		IIC_REG_ERR_RETURN(IICReg_clearBitMask(&pDrv->iicReg, BitFramingReg, 0x80));//关闭发送

		SwTimer_Stop(&pDrv->timer);
		pDrv->transStatus = TRANSFER_STATUS_IDLE;
	}
}	

/*********************************************/
/*函数名：	    Set_RF  */
/*功能：	    设置射频输出    */
				
/*输入参数：	mode，射频输出模式  
				0，关闭输出
				1,仅打开TX1输出
				2,仅打开TX2输出
				3，TX1，TX2打开输出，TX2为反向输出  */
/*返回值：	    OK
				ERROR   */
/*********************************************/
Bool fm175Drv_setRf(Fm175Drv* pDrv, unsigned char mode)
{
    unsigned char result;
    
	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(&pDrv->iicReg, TxControlReg, &result));
	if ((result & 0x03) == mode) return True;

	if (mode == 0) IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxControlReg, 0x03));	 //关闭TX1，TX2输出
	if (mode == 1) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //仅打开TX1输出
	if (mode == 2) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //仅打开TX2输出
	if (mode == 3) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //打开TX1，TX2输出
	
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

	//循环读取所有FIFO的内容
	for (; item->transBufOffset < item->transLen; )
	{
		int readLen = MIN(item->transBufLen - item->transBufOffset, item->transLen - item->transBufOffset);
		if (readLen == 0)
		{
			fm175Drv_event(pDrv, TRANS_RX_BUF_FULL, TRANS_RESULT_SUCCESS);
			item->transBufOffset = 0;
			continue;
		}

		IIC_REG_ERR_RETURN(IICReg_readFifo(&pDrv->iicReg, &pDrv->rxBuf[item->transBufOffset], readLen)); //读出FIFO内容
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

	//当前还有多少未传输的数据在FIFO
	uint8 fifeSize;
	IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &fifeSize));

	//计算实际已经传输的数据
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
		//用户应该在此事件中，复制要传输的数据到txBuf中，并且设置item->transBufLen的值
		fm175Drv_event(pDrv, TRANS_TX_BUF_EMPTY, TRANS_RESULT_SUCCESS);
		item->transBufOffset = 0;
	}

	//计算发送多少字节到FIFO中
	item->transLen = MIN(item->transBufLen - item->transBufOffset, cfg->fifoDeepth - fifeSize);

	if (item->transLen)
	{
		IIC_REG_ERR_RETURN(IICReg_writeFifo(&pDrv->iicReg, & pDrv->txBuf[item->transBufOffset], item->transLen));
		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, BitFramingReg, 0x80));	//启动发送
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
	//不处理，Mifare卡片的写操作时会返回4个BIT,需要屏蔽错误检测
}
void fm175Drv_irq_timeOut(Fm175Drv* pDrv)
{
	if (pDrv->transStatus == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//获取中断处理函数
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

			//清中断
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

		//打开TX输出后需要延时等待天线载波信号稳定
		SwTimer_Start(&pDrv->timer, 1, 0);
	}
	else if (state == FM_STATE_SLEEP)
	{
		//进入低功耗
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
	//同步发送方式，发完数据等待接收
	uint8 irq = 0;
	while (pDrv->transStatus != TRANSFER_STATUS_IDLE && !SwTimer_isTimerOut(&pDrv->timer))
	{
		if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//查询中断标志	
		{
			fm175Drv_irq(pDrv, irq);
		}
	}
}

void fm175Drv_fsmActive(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	//是否发送命令超时，没有接收到响应
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
			//同步发送方式，发完数据等待接收
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

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	pDrv->fsm(pDrv, msg, param);
}

#endif

/*********************************************/
/*函数名：	    fm175Drv_setTimer    */
/*功能：	    设置接收延时    */
/*输入参数：	delaytime，延时时间（单位为毫秒）    */
/*返回值：	    OK    */
/*********************************************/
Bool fm175Drv_setTimer(Fm175Drv* pDrv, unsigned long delaytime)//设定超时时间（ms）
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
函数功能：传输初始化,如果是大块数据传输，需要分批填充txBuf,可在设置该函数中的参数txBuf=Null。
参数说明：
	pDrv：确定程序对象。
	txBuf： 发送数据指针,如果为NULL，表示在此不指定传输数据(传输大块数据)，需要再TRANS_TX_BUF_EMPTY中填充发送数据
	txBufSize： 发送数据长度
	rxBuf：		接收数据指针
	rxBufSize： 接收数据长度
返回值：TRUE-传输初始化成功，FALSE-传输初始化失败
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj)
{
	TransMgr* item = &pDrv->transMgr;
	//有数据正在传输
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
		//如果txBuf为NULL，表示不指定传输数据，需要在TRANS_TX_BUF_EMPTY中填充发送数据
		item->transBufLen = 0;
		item->totalLen = pDrv->txBufSize;
	}
	pDrv->rxBuf		= rxBuf;
	pDrv->rxBufSize = rxBufSize;
	pDrv->obj = pObj;

	return True;
}


/***************************************************************
函数功能：传输启动，启动后会触发传输事件，通过调用事件回调函数通知传输结果，成功或者失败，参考事件定义FM_EVENT。
参数说明：
	pDrv：确定程序对象。
	cmd：传输命令
	timeOutMs： 超时时间
返回值：TRUE-传输启动成功，FALSE-传输启动失败
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

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->cfg->waterLevel));//设置WaterLeve
//   	rx_temp = IICReg_readByte(&pDrv->iicReg, WaterLevelReg);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, 0x7F));//清除IRQ标志
//	rx_temp = IICReg_readByte(&pDrv->iicReg, ComIrqReg);

	IIC_REG_ERR_RETURN_FALSE(fm175Drv_setTimer(pDrv, timeOutMs));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, 0x80));//自动启动定时器
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, cmd));

	pDrv->transStatus = TRANSFER_STATUS_TX;
	SwTimer_Start(&pDrv->timer, timeOutMs + 10, 0);

	fm175Drv_event(pDrv, TRANS_TX_START, TRANS_RESULT_SUCCESS);

	fm175Drv_irq_tx(pDrv);

	return True;
}

//同步发送数据
Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
)
{
	//如果设备状态state是激活，
	if (pDrv->state == FM_STATE_ACTIVE)
	{
		//传输状态transStatus为非TRANSFER_STATUS_IDLE，返回失败
		if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	}
	else if (pDrv->state == FM_STATE_SEARCH_CARD)
	{
		//如果设备状态state是搜卡，优先级最高，传输状态transStatus为任何状态都可以发送
	}
	else //其他状态，返回失败
	{
		return False;
	}

	int rc = False;

	//临时保存传输参数，同步传输结束后恢复**************************
	TransEventFn	_evtFn		= pDrv->Event	;
	FM17522_CMD		_cmd		= pDrv->cmd		;
	const void*		_txBuf		= pDrv->txBuf	;
	int				_txBufSize  = pDrv->txBufSize;
	void*			_rxBuf      = pDrv->rxBuf	;
	int				_rxBufSize  = pDrv->rxBufSize;
	/**************************************************************/

	pDrv->Event = Null;	//事件函数指针置空,同步传输不需要Event调用

	//启动传输并等待结果
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
	//恢复传输参数
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

void fm175Drv_init(Fm175Drv* pDrv, uint8 iicAddr, const TransProtocolCfg* cfg, TransEventFn	Event)
{
	memset(pDrv, 0, sizeof(Fm175Drv));

	pDrv->cfg = cfg;
	pDrv->transStatus = TRANSFER_STATUS_IDLE;
	pDrv->antPort = 1;
	pDrv->Event = Event;

	pDrv->iicReg.dev_addr = iicAddr;
}
