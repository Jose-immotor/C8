
#include "Common.h"
#include "NfcCardReader.h"
#include "fm175xx.h"
#include "type_a.h"
#include "fm175Drv.h"

#define ERROR		1
#define	OK			0

static Fm175Drv g_FmDrv;
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

	fm175Drv_switchPort(&g_FmDrv, port);

	return (pReader->latestErr == OK);
}

static void NfcCardReader_fsm_trans(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen && fm175Drv_isIdle(&g_FmDrv))
		{
			//发送数据
			PFL(DL_NFC,"NFC send data length:%d!\n",pReader->txLen);
			fm175Drv_transStart(&g_FmDrv, FM_CMD_TRANSCEIVE
				, pReader->txBuf, pReader->txLen
				, pReader->rxBuf, sizeof(pReader->rxBuf)
				, 300, pReader
			);
		}
		else if(SwTimer_isTimerOut(&pReader->sleepTimer))
		{
			//如果空闲超时，则切换到Sleep模式
			NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
		}
	}
	else if(msgId == CARD_READER_MSG_TX_DONE)
	{
		PFL(DL_NFC, "NFC send data [%s]\n", param ? "SUCCESS" : "FAILED");
		if (param)
		{
			pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
			NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
		}
		else
		{
			pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_SUCCESS);

		}
		pReader->rxLen = 0;
		pReader->txLen = 0;

		//启动定时器
		SwTimer_Start(&pReader->sleepTimer, 60000, 0);
	}
}

static void NfcCardReader_fsm_sleep(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen )
		{
			//同一端口2次搜卡的最小间隔是10MS
			if (SwTimer_isTimerOutEx(pReader->searchTicks, 10) && g_FmDrv.state == FM_STATE_WORK)
			{
				NfcCardReader_searchPort(pReader, pReader->port);
			}
		}
	}
	else if (CARD_READER_MSG_SEARCH_CARD_DONE == msgId)
	{
		pReader->searchTicks = GET_TICKS();

		if (g_FmDrv.cardIsExist)
		{
			NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_trans);
		}
		else if(pReader->port == 0)
		{
			pReader->port = 1;	//搜索第二个Port
		}
		else
		{
			pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
			pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_FAILED);

			pReader->port = 0;
			pReader->rxLen = 0;
			pReader->txLen = 0;
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

void NfcCardReader_postMsg(NfcCardReader* pReader, uint8 msgId, uint8 param1)
{

}

void NfcCardReader_Fm175Drv_EventCb(NfcCardReader* pReader, TRANS_EVENT evt)
{
	if (evt == TRANS_SUCCESS)
	{
		pReader->txLen = 0;
		pReader->rxLen = g_FmDrv.transMgr.totalLen;
		NfcCardReader_postMsg(pReader, CARD_READER_MSG_TX_DONE, True);
	}
	else if (evt == TRANS_FAILED)
	{
		pReader->txLen = 0;
		pReader->rxLen = 0;
		NfcCardReader_postMsg(pReader, CARD_READER_MSG_TX_DONE, False);
	}
	else if (evt == SEARCH_CARD_DONE)
	{
		NfcCardReader_postMsg(pReader, CARD_READER_MSG_SEARCH_CARD_DONE, g_FmDrv.cardIsExist);
	}
}

void NfcCardReader_init(NfcCardReader* pReader, NfcCardReader_EventFn Event, void* cbObj)
{
	//memset(pReader, 0, sizeof(NfcCardReader));

	//pReader->status = NfcCardReaderStatus_unknown;
	//pReader->Event = Event;
	//pReader->cbObj = cbObj;
	//
	//FM17522_Init();

	static const TransProtocolCfg cfg =
	{
		.waterLevel = 32,
		.fifoDeepth = 64,
		.Event = (TransEventFn)NfcCardReader_Fm175Drv_EventCb
	};
	fm175Drv_init(&g_FmDrv, &cfg, pReader);
}

