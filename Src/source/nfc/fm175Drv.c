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
#define FM17522_NPD_HIGHT	//待定
#define FM17522_NPD_LOW		//待定


/*********************************************/
/*函数名：	    Pcd_ConfigISOType    */
/*功能：	    设置操作协议    */
/*输入参数：	type 0，ISO14443A协议；    */
/*					 1，ISO14443B协议；    */
/*返回值：	    OK    */
/*********************************************/
void fm175Drv_ConfigISOType(unsigned char type)
{
	if (type == 0)                     //ISO14443_A
	{
		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式// chenke, FM17522 的bit4是RFT预留       
		Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 设置100%ASK有效        
		Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A// chenke, TX CRC无效, TXSPEED 106Kbit        
		Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A// chenke, RX CRC无效, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
	}
	if (type == 1)                     //ISO14443_B
	{
		Write_Reg(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式        
		Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 设置TX CRC有效，TX FRAMING =TYPE B        
		Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 设置RX CRC有效，RX FRAMING =TYPE B        
		Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 设置ON电导        
		Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 设置OFF电导        
		Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 设置100%ASK无效        
	}
}

void fm175Drv_event(Fm175Drv* pDrv, FM_EVENT evt, TRANSFER_RESULT res)
{
	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		Set_BitMask(ControlReg, 0x80);           // stop timer now
		Write_Reg(CommandReg, Idle);
		Clear_BitMask(BitFramingReg, 0x80);//关闭发送

		SwTimer_Stop(&pDrv->timer);
		pDrv->status = TRANSFER_STATUS_IDLE;
	}

	if (pDrv->cfg->Event) pDrv->cfg->Event(pDrv->obj, evt);
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
void fm175Drv_setRf(unsigned char mode)
{
    unsigned char result;
    
    result = Read_Reg(TxControlReg);
    
    if	((result&0x03) == mode)//chenke, 判断是否TX1和TX2已经输出
    {
    }
    if (mode==0)
	{
	    result=Clear_BitMask(TxControlReg,0x03); //关闭TX1，TX2输出
	}
    if (mode==1)
	{
    	result=Set_BitMask(TxControlReg,0x01); //仅打开TX1输出
	}
    if (mode==2)
	{
    	result=Set_BitMask(TxControlReg,0x02); //仅打开TX2输出
	}
    if (mode==3)
	{
    	result=Set_BitMask(TxControlReg,0x03); //打开TX1，TX2输出
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

		//打开TX输出后需要延时等待天线载波信号稳定
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

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

void fm175Drv_fsm(Fm175Drv* pDrv)
{
	pDrv->fsm(pDrv);
}

#endif

/*********************************************/
/*函数名：	    fm175Drv_setTimer    */
/*功能：	    设置接收延时    */
/*输入参数：	delaytime，延时时间（单位为毫秒）    */
/*返回值：	    OK    */
/*********************************************/
void fm175Drv_setTimer(unsigned long delaytime)//设定超时时间（ms）
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
函数功能：传输启动，启动后会触发传输事件，通过调用事件回调函数通知传输结果，成功或者失败，参考事件定义FM_EVENT。
参数说明：
	pDrv：确定程序对象。
	cmd：传输命令
	txBuf： 发送数据指针
	txBufSize： 发送数据长度
	rxBuf：		接收数据指针
	rxBufSize： 接收数据长度
	timeOutMs： 超时时间
	cbObj：		事件回调函数的对象参数
返回值：TRUE-传输启动成功，FALSE-传输启动失败
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

	Write_Reg(WaterLevelReg, pDrv->cfg->waterLevel);//设置WaterLeve
//   	rx_temp = Read_Reg(WaterLevelReg);

	Write_Reg(ComIrqReg, 0x7F);//清除IRQ标志
//	rx_temp = Read_Reg(ComIrqReg);

	fm175Drv_setTimer(timeOutMs);
	Set_BitMask(TModeReg, 0x80);//自动启动定时器
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

		Read_FIFO(readLen, &pDrv->rxBuf[item->transBufOffset]); //读出FIFO内容
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
	int fifeSize = Read_Reg(FIFOLevelReg);
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
		Write_FIFO(item->transLen, (uint8*)&pDrv->txBuf[item->transBufOffset]);
		Set_BitMask(BitFramingReg, 0x80);	//启动发送
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
	if (pDrv->status == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//获取中断处理函数
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
	uint8 irq = Read_Reg(ComIrqReg);//查询中断标志		

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
