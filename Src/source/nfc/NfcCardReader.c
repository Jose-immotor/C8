
#include "Common.h"
#include "NfcCardReader.h"
#include "fm175xx.h"
#include "type_a.h"

#define ERROR		1
#define	OK			0

static NfcCardReader_FsmFn NfcCardReader_findStatusProcFun(NfcCardReader* pReader, NfcCardReaderStatus status);

void NfcCardReader_dump(const NfcCardReader* pReader)
{
	Printf("cardReader:\n");
	Printf("\t port=%d.\n", pReader->port);
	Printf("\t status=%d.\n", pReader->status);
	//	Printf("\t Fsm=%d.\n"	, pPkt->Fsm);
}

static void NfcCardReader_switchStatus(NfcCardReader* pReader, NfcCardReaderStatus status)
{
	if (status == pReader->status) return;

	if (status == NfcCardReaderStatus_sleep)
	{
		//进入低功耗
		pReader->Event(pReader->cbObj, CARD_EVENT_SLEEP);
		FM175XX_SoftPowerdown();
		pReader->port = 0;
	}
	else if (status == NfcCardReaderStatus_trans)
	{
	}
	PFL(DL_NFC,"NFC status from %d to %d!\n",pReader->status,status);
	pReader->status = status;

	//修改状态机函数指针
	pReader->Fsm = NfcCardReader_findStatusProcFun(pReader, status);
}

//搜卡
static Bool NfcCardReader_searchPort(NfcCardReader* pReader, int port)
{
	//记录搜索时间和次数
	pReader->searchTicks = (pReader->port != port) ? 0 : GET_TICKS();
	pReader->searchCounter = (pReader->port != port) ? 0 : (pReader->searchCounter + 1);

//	pReader->port = port;

	
	FM175XX_switchPort(port);
	FM175XX_SoftReset();
//	Write_Reg(WaterLevelReg,0x20);//设置FIFOLevel=32字节  
//	Write_Reg(DivIEnReg,0x80);//引脚IRQ按照标准CMOS输出pad工作
//	Write_Reg(ComIEnReg,0x2c);//允许RxIEn,HiAlerlEn,LoAlertlEn
	if (pReader->port)
	{
		Set_Rf(1);
	}
	else
	{
		Set_Rf(2);
	}
	Pcd_ConfigISOType(0);
	PFL(DL_NFC,"NFC port[%d] search start!\n",pReader->port);
	pReader->latestErr = TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK);

	if (pReader->latestErr == OK)
	{
		pReader->latestErr = TypeA_RATS(0x20, pReader->picc_ats);
	}

	return (pReader->latestErr == OK);
}

static void NfcCardReader_fsm_trans(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen)
		{
			//设定超时事件
			Pcd_SetTimer(300);
			//发送数据
			PFL(DL_NFC,"NFC send data length:%d!\n",pReader->txLen);
			//FM17522_Delayms(10);//程序延迟
			pReader->latestErr = Pcd_Comm(Transceive, pReader->txBuf, pReader->txLen, pReader->rxBuf, &pReader->rxLen);
			PFL(DL_NFC,"NFC send data error:%d(0-ok,1-err)!\n",pReader->latestErr);
			if (pReader->latestErr != OK)
			{
				pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
				NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
			}
			else
			{
				pReader->rxLen = (pReader->rxLen +7)/8;
				pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_SUCCESS);

				//启动定时器
				SwTimer_Start(&pReader->sleepTimer, 60000, 0);
			}
			pReader->rxLen = 0;
			pReader->txLen = 0;
		}
		else if(SwTimer_isTimerOut(&pReader->sleepTimer))
		{
			//如果空闲超时，则切换到Sleep模式
			NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
		}
	}
}

static void NfcCardReader_fsm_sleep(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	Bool isOk;
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen )
		{
			//同一端口2次搜卡的最小间隔是10MS
			if (SwTimer_isTimerOutEx(pReader->searchTicks, 10))
			{
				//搜卡
				if (!NfcCardReader_searchPort(pReader, pReader->port))
				{
					if (pReader->port == 0)
					{
						pReader->port = 1;	//搜索第二个Port
					}
					else //2个卡都搜索失败
					{
						pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
						pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_FAILED);

						pReader->port = 0;
						pReader->rxLen = 0;
						pReader->txLen = 0;
					}
				}
				else
				{
					NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_trans);
				}
			}
		}
	}
}

//查找状态响应的处理函数
static NfcCardReader_FsmFn NfcCardReader_findStatusProcFun(NfcCardReader* pReader, NfcCardReaderStatus status)
{
	struct
	{
		NfcCardReaderStatus opStatus;
		NfcCardReader_FsmFn Fsm;
	}
	const static g_dispatch[] =
	{
		{NfcCardReaderStatus_sleep	, NfcCardReader_fsm_sleep},
		{NfcCardReaderStatus_trans	, NfcCardReader_fsm_trans},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_dispatch); i++)
	{
		if (g_dispatch[i].opStatus == pReader->status)
		{
			return g_dispatch[i].Fsm;
		}
	}
	return Null;
}

static void NfcCardReader_fsm(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	//All status do...

	if (pReader->Fsm)
	{
		pReader->Fsm(pReader, msgId, param);
	}
}

Bool NfcCardReader_isIdle(NfcCardReader* pReader)
{
	return pReader->txLen == 0;
}

Bool NfcCardReader_Send(NfcCardReader* pReader, const void* data, int len)
{
	//待发数据没有发送完毕
	if (pReader->txLen) return False;
	//检验发送长度
	if (len > sizeof(pReader->txBuf)) return False;

	memcpy(pReader->txBuf, data, len);
	pReader->txLen = len;

	return True;
}

void NfcCardReader_reset(NfcCardReader* pReader)
{
	
}

void NfcCardReader_run(NfcCardReader* pReader)
{
	NfcCardReader_fsm(pReader, CARD_READER_MSG_RUN, 0);
}

void NfcCardReader_thread_entry(void* pReader)
{
	NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
	while (1)
	{
		NfcCardReader_fsm(pReader, CARD_READER_MSG_RUN, 0);
		rt_thread_mdelay(1);
	}
}

void NfcCardReader_start(NfcCardReader* pReader)
{
#ifdef USE_NFC_THREAD
	rt_thread_t nfc_task_tid = rt_thread_create("NfcCardReader",/* 线程名称 */
		NfcCardReader_thread_entry, pReader,
		2048, 3, 10); //
	rt_thread_startup(nfc_task_tid);
#else
	NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
#endif
}

void NfcCardReader_init(NfcCardReader* pReader, NfcCardReader_EventFn Event, void* cbObj)
{
	memset(pReader, 0, sizeof(NfcCardReader));

	pReader->status = NfcCardReaderStatus_unknown;
	pReader->Event = Event;
	pReader->cbObj = cbObj;
	
	FM17522_Init();
}
