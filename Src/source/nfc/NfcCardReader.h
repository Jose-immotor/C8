#ifndef __NFC_CARD_READER__H_
#define __NFC_CARD_READER__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "queue.h"
#include "SwTimer.h"
	//读卡器命令
	typedef enum _NfcCardReaderMsg
	{
		CARD_READER_MSG_RUN = 0X01,		//运行
		CARD_READER_MSG_TIMEOUT,		//超时
		CARD_READER_MSG_TX,				//发送数据
		CARD_READER_MSG_TX_DONE,		//发送数据借宿
		CARD_READER_MSG_WAKEUP,			//唤醒
		CARD_READER_MSG_SEARCH_CARD_DONE,		//搜卡结束
		CARD_READER_MSG_SEARCH_PORT,	//搜索串口,消息参数：CARD_READER_MSG_SEARCH_PORT(port)
	}NfcCardReaderMsg;

	//读卡器时间
	typedef enum _NfcCardReaderEvent
	{
		CARD_EVENT_SEARCH_SUCCESS = 0X01,//搜卡成功
		CARD_EVENT_SEARCH_FAILED,		//搜卡失败
		CARD_EVENT_RX_DATA_SUCCESS,		//接收到数据成功，事件参数：(port, rxBuf, rxLen)
		CARD_EVENT_RX_DATA_FAILED,		//接收到数据失败，失败原因码latestErr；
		CARD_EVENT_TX_DATA_SUCCESS,		//接收到数据成功，事件参数：(port, txBuf, txLen)
		CARD_EVENT_TX_DATA_FAILED,		//接收到数据失败，失败原因码latestErr；
		CARD_EVENT_SLEEP,				//休眠
		CARD_EVENT_WAKEUP,				//唤醒
	}NfcCardReaderEvent;

	typedef enum _NfcCardReaderEventRc
	{
		CARD_EVENT_RC_SUCCESS = 0,	//成功
		CARD_EVENT_RC_FAILED,		//失败
	}NfcCardReaderEventRc;

	//读卡器操作状态
	typedef enum _NfcCardReaderStatus
	{
		NfcCardReaderStatus_sleep,		//低功耗
		NfcCardReaderStatus_trans,		//传输
		NfcCardReaderStatus_unknown,	//初始值
	}NfcCardReaderStatus;

	struct _NfcCardReader;
	typedef NfcCardReaderEventRc(*NfcCardReader_EventFn)(void* pObj, NfcCardReaderEvent ev);
	typedef void (*NfcCardReader_FsmFn)(struct _NfcCardReader* pReader, uint8_t msgId, uint32_t param);

	typedef struct _NfcCardReader
	{
		uint8_t port;	//天线ID，从0开始
		uint8_t searchCounter;	//搜索同一端口计数器
		uint32_t searchTicks;	//搜索端口Ticks,用于计算搜索同一端口的时间间隔

		SwTimer sleepTimer;
		//unsigned char picc_ats[14];

		NfcCardReaderStatus status;

		//状态机函数指针，由status决定指向那个状态处理函数
		NfcCardReader_FsmFn Fsm;

		uint8_t rxBuf[128];	//接收缓存区
		uint32_t rxLen;		//接收缓存区数据长度
		uint8_t txBuf[128];	//发送缓存区
		uint32_t txLen;		//发送缓存区数据长度

		uint8 latestErr;

		//接收数据处理回调函数
		NfcCardReader_EventFn Event;
		//回调函数对象，被回调函数使用，来自初始化函数
		void* cbObj;
	}NfcCardReader;

	void NfcCardReader_dump(const NfcCardReader* pReader);
	void NfcCardReader_init(NfcCardReader* pReader, NfcCardReader_EventFn Event, void* cbObj);
	Bool NfcCardReader_isIdle(NfcCardReader* pReader);
	Bool NfcCardReader_Send(NfcCardReader* pReader, const void* data, int len);
	void NfcCardReader_run(NfcCardReader* pReader);
	void NfcCardReader_start(NfcCardReader* pReader);
#ifdef __cplusplus
}
#endif

#endif

