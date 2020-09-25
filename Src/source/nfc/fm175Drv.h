#if 1

#ifndef FM175XX_DRIVER_H
#define FM175XX_DRIVER_H

#include "typedef.h"
#include "Transfer.h"
#include "Bit.h"
#include "SwTimer.h"
#include "fm175Reg.h"

typedef enum _NFC_ISO_TYPE
{
	NFC_ISO_TYPE_A = 0,	//ISO14443_A
	NFC_ISO_TYPE_B = 1,	//ISO14443_B
}NFC_ISO_TYPE;

typedef enum _FM17522_MSG
{
	FM_MSG_RUN,
	FM_MSG_SWITCH_NFC,	//Fsm(FM_MSG_SWITCH_NFC, iicAddr)
}FM17522_MSG;

//FM17522传输命令码定义
typedef enum _FM17522_CMD
{
	FM_CMD_IDLE			=0x00 ,
	FM_CMD_MEM			=0x01 ,
	FM_CMD_RANDOMID		=0x02 ,
	FM_CMD_CALCCRC		=0x03 ,
	FM_CMD_TRANSMIT		=0x04 ,
	FM_CMD_NOCMDCHANGE	=0x07 ,
	FM_CMD_RECEIVE		=0x08 ,
	FM_CMD_TRANSCEIVE	=0x0C ,
	FM_CMD_MFAUTHENT	=0x0E ,
	FM_CMD_SOFTRESET	=0x0F ,
}FM17522_CMD;

//工作状态定义
typedef enum _FM17522_STATE
{
	FM_STATE_INIT = 0x01,
	FM_STATE_NPD_LOW,
	FM_STATE_NPD_HIGH,
	FM_STATE_SEARCH_CARD,	//搜卡状态
	FM_STATE_SLEEP,			//休眠，低功耗
	FM_STATE_TRANSFER,		//搜卡成功,传输状态
}FM17522_STATE;

#define REG_IRQ_MASK_TX			BIT_6
#define REG_IRQ_MASK_RX			BIT_5
#define REG_IRQ_MASK_IDLE		BIT_4
#define REG_IRQ_MASK_HI_ALERT	BIT_3
#define REG_IRQ_MASK_LO_ALERT	BIT_2
#define REG_IRQ_MASK_ERR		BIT_1
#define REG_IRQ_MASK_TIMEOUT	BIT_0

struct _Fm175Drv;
typedef void (*FmIrqFn)(struct _Fm175Drv* pDrv);

typedef struct _REG_CommIRq
{
	uint8 Set1 : 1;			//BIT[7]
	uint8 TxDoneIrq : 1;	//BIT[6]
	uint8 RxDoneIrq : 1;	//BIT[5]
	uint8 IdleIrq : 1;		//BIT[4]
	uint8 HiAlertIrq : 1;	//BIT[3]
	uint8 LoAlertIrq : 1;	//BIT[2]
	uint8 ErrIrq : 1;		//BIT[1]
	uint8 TimeOutIrq : 1;	//BIT[0]
}REG_CommIRq;

struct _Fm175Drv;
typedef void (*FmFsmFn)(struct _Fm175Drv* p, FM17522_MSG msg, uint32 param);
typedef struct _Fm175Drv
{
	FM17522_STATE state;		//工作状态
	uint8 antPort;				//天线号
	Bool cardIsExist;			//卡是否存在

	FM17522_CMD cmd;				//命令码
	TRANSFER_STATUS transStatus;	//传输状态

	const uint8_t* txBuf;		//发送数据Buffer
	int txBufSize;		//发送数据Buffer长度

	uint8_t* rxBuf;		//接收数据Buffer
	int rxBufSize;		//接收数据长度,

	/*************************************************
	接收数据时：如果 FIFO 缓冲器空间剩下的字节数小于等于 WaterLevel 定义的字节数， 触发HiAlert中断
	发送数据时：如果 FIFO 中的字节数小于等于 WaterLevel 字节数， 触发LoAlert中断
	*************************************************/
	uint8 waterLevel;

	const TransProtocolCfg* cfg;//传输协议配置
	TransMgr transMgr;			//传输管理

	SwTimer sleepWdTimer;		//睡眠看门狗定时器，如果不喂狗，则超时会进入睡眠
	SwTimer timer;				//发送命令时定时器

	TRANSFER_RESULT latestErr;	//传输的最后错误码

	IICReg iicReg;	//IIC寄存器对象

	FmFsmFn fsm;	//状态机函数

	void* obj;				//用户事件回调函数对象
	TransEventFn	Event;	//用户事件回调函数指针
}Fm175Drv;

/***************************************************************
函数功能：传输启动，启动后会触发传输事件，通过调用事件回调函数通知传输结果，参考事件定义FM_EVENT。
参数说明：
	pDrv：确定程序对象。
	txBuf： 发送数据指针
	txBufSize： 发送数据长度
	rxBuf：		接收数据指针
	rxBufSize： 接收数据长度
	cbObj：		事件回调函数的对象参数
返回值：TRUE-传输初始化成功，FALSE-传输初始化失败
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj);

Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
);

inline Bool fm175Drv_isIdle(const Fm175Drv* pDrv) { return pDrv->transStatus == TRANSFER_STATUS_IDLE; };

void fm175Drv_init(Fm175Drv* pDrv, uint8 iicAddr, const TransProtocolCfg* cfg, TransEventFn	Event);
void fm175Drv_run(Fm175Drv* pDrv);
void fm175Drv_start(Fm175Drv* pDrv);
Bool fm175Drv_switchNfc(Fm175Drv* pDrv, uint8 iicAddr);
void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param);

#endif

#endif


