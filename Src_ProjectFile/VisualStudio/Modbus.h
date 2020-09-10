#pragma once
/******************************************************

具体格式定义如下：

 |-----|----|------|---------|----|
  add   cmd   reg      count  crc

**********************************************************/

#ifndef __MODBUS__H_
#define __MODBUS__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "SwTimer.h"

	//重发标志，无限次重发
#define MOD_ENDLESS 0xFF

#define MODBUS_CMD_IND  1
#define MODBUS_LEN_IND  2
#define MODBUS_DATA_IND 3


	typedef enum
	{
		MOD_CMD_READ_COIL_STATUS = 0X01,	//读线圈状态,，
		MOD_CMD_READ_INPUT_STATUS = 0x02,		//读离散输入寄存器
		MOD_CMD_READ_HOLDING_REG = 0x03,		//读保持寄存器
		MOD_CMD_READ_INPUT_REG = 0x04,		//读输入寄存器
		MOD_CMD_WEITE_SINGLE_COIL = 0x05,		//写单个线圈
		MOD_CMD_WEITE_SINGLE_REG = 0x06,		//写单个寄存器
		MOD_CMD_READ_EXCEPTION_STATUS = 0x07, //读异常状态

		MOD_CMD_WEITE_MULTI_COIL = 0x0F,	//写多个线圈
		MOD_CMD_WEITE_MULTI_REG = 0x10,			//写多个寄存器
	}MOD_CMD;

	/***************
	传输事件定义
	***************/
	typedef enum _MOD_TXF_EVENT
	{
		MOD_TX_START = 0	//发送命令开始, 可在该事件中配置通信参数：waitRspMs, maxTxCount
		, MOD_CHANGED_BEFORE	//pStorage值改变之前
		, MOD_CHANGED_AFTER	//pStorage值改变之后
		, MOD_GET_RSP		//接收到请求,获取响应，响应数据在transferData, transferLen中
		, MOD_REQ_FAILED		//请求结束，失败，原因：可能传输超时，或者响应返回失败码。
		, MOD_REQ_SUCCESS	//请求结束，成功，传输成功并且响应返回成功码
	}MOD_TXF_EVENT;

	typedef enum
	{
		MOD_EVENT_RC_SUCCESS = 0	//事件处理成功
		, MOD_EVENT_RC_FAILED		//事件处理失败
		, MOD_EVENT_RC_DO_NOTHING	//事件没有处理
	}MOD_EVENT_RC;

	//MOD 传输状态
	typedef enum
	{
		MOD_FSM_INIT = 0	//初始化
		, MOD_FSM_RX_REQ	//接收到请求
		, MOD_FSM_WAIT_RSP	//等待响应
	}MOD_FSM;

	typedef enum
	{
		MOD_READ = 0	//读命令类型，需要应答
		, MOD_WRITE		//写命令类型，需要应答
		, MOD_NOTIFY	//通知类型  ，不需要应答
	}MOD_CMD_TYPE;

	typedef enum _MOD_RSP_RC
	{
		MOD_RSP_SUCCESS = 0		//接收响应成功
		, MOD_RSP_TIMEOUT		//接收响应超时，对方没响应
		, MOD_RSP_CANCEL		//取消发送
	}MOD_RSP_RC;

	//函数类型定义
	struct _ModCmd;
	struct _Mod;
	typedef MOD_EVENT_RC(*ModEventFn)(void* pOwnerObj, const struct _ModCmd* pCmd, MOD_TXF_EVENT ev);
	typedef int (*ModTxFn)(const uint8_t* pData, int len);

	//Mod帧配置特征
	typedef struct _ModFrame
	{
		uint8_t  addr;	//txBuf的长度
		uint8_t  cmd;		//存放发送出去数据，转码前
		int      rxBufLen;	//rxBuf的长度
		uint8_t* rxBuf;		//存放接收到的RAW数据，转码前

		uint32_t waitRspMsDefault;	//命令的默认的等待响应时间，如果命令要修改为非默认值，可以在命令的事件函数MOD_TX_START中修改pMod->waitRspMs
		uint32_t rxIntervalMs;		//接收数据间隔
		uint32_t sendCmdIntervalMs;	//发送2个命令之间的间隔时间
	}ModFrame;


	/*********************************
	函数功能：帧校验，实现2个功能；
		1）如果req=Null,仅仅检验接收到的数据是否完整。
		2）如果req!=Null,检验接收的数据帧是否和Req匹配。
		3）如果协议中包含Seq字节和Addr字节，也可以使用该函数检验和过滤
	参数说明：
		pMod：MOD对象指针。
		rxFrame：要检验的数据帧。
		frameLen：帧长度。
		req：请求帧，如果req=Null，表示不需要检验Req
	返回值：校验结果。
	***********************************/
	typedef Bool(*ModFrameVerifyFn)(struct _Mod* pMod, const uint8_t* rxFrame, int frameLen, const uint8_t* req);

	/*********************************
	函数功能：把要发送的数据数据打包成为一个请求数据帧或者响应帧，由参数reqFrame决定。
	参数说明：
		pMod：MOD对象指针。
		cmd：命令码。
		pData：数据指针。
		len：数据长度
		reqFrame：如果=NULL，表示根据请求帧打包响应帧；如果!=NULL，表示打包一个请求帧。
		rspFrame：输出参数，要打包的数据帧指针
	返回值：数据帧长度。
	***********************************/
	typedef int (*ModBuildFrameFn)(struct _Mod* pMod, uint8_t cmd, const void* pData, int len, const uint8* reqFrame, uint8* rspFrame);

	//Mod帧配置特征
	typedef struct _ModFrameCfg
	{

		int      txBufLen;	//txBuf的长度
		uint8_t* txBuf;		//存放发送出去数据，转码前
		int      rxBufLen;	//rxBuf的长度
		uint8_t* rxBuf;		//存放接收到的RAW数据，转码前

		//uint8_t result_SUCCESS;		//返回码定义：成功
		//uint8_t result_UNSUPPORTED;	//返回码定义：接收到不支持的请求

		uint32_t waitRspMsDefault;	//命令的默认的等待响应时间，如果命令要修改为非默认值，可以在命令的事件函数MOD_TX_START中修改pMod->waitRspMs
		uint32_t rxIntervalMs;		//接收数据间隔
		uint32_t sendCmdIntervalMs;	//发送2个命令之间的间隔时间

	//	ModFrameVerifyFn FrameVerify;	//帧校验函数
	//	ModBuildFrameFn	 FrameBuild;	//帧打包函数
	}ModFrameCfg;

	//Mod其他配置项
	typedef struct _ModCfg
	{
		uint8_t  port;	//电池端口好，从0开始
		int					  cmdCount;	//命令数组总数
		const struct _ModCmd* cmdArray;	//命令数组

		ModTxFn		TxFn;				//数据的发送到总线的函数指针
		void* pCbObj;				//回调函数参数
		ModEventFn	TresferEvent;		//事件回调函数
	}ModCfg;

	typedef struct _ModCmdFmt
	{
		uint8_t dataByteInd;
		uint8_t lenByteInd;
		uint8_t regByteInd;
	}ModCmdFmt;

	typedef struct _ModCmdEx
	{
		/*
		如果是EVENT，
			在MOD_GET_RSP事件之前transferData指向接收到的数据
			在MOD_GET_RSP事件中transferData默认指向ModCmd->pData，也可以
				在事件函数中修改，指向要应答的数据指针，由上层程序决定
		如果是READ/WRITE,指向应答数据
		*/
		const uint8_t* transferData;	//传输数据指针
		uint8_t transferLen;		//传输数据长度

		/*****************
		发送请求或接收响应时的时间戳Ticks.
		对于MOD_READ/MOD_WRITE类型：保存接收响应时的Ticks：
		对于MOD_EVENT类型：无效
		对于MOD_NOTIFY类型：发送请求时的Ticks。
		******************/
		uint32_t rxRspTicks;

		/*****************
		发送标志:
			0：表示不发送，
			其他值：表示延时指定的时间MS发送
		******************/
		uint32_t sendDelayMs;

		MOD_RSP_RC rcvRspErr;	//接收响应错误码

		Bool isForceSend;		//强制发送标志
	}ModCmdEx;

	typedef struct _ModCmd
	{
		ModCmdEx* pExt;	//ModCmd扩展
		uint8_t cmd;	//命令ID
		MOD_CMD_TYPE type;	//读或者写命令，参考MOD_CMD_TYPE定义
		uint8_t modCmd;			//命令码
		const char* cmdName;//命令名称

		//如果是READ，保存读回来的数据
		//如果是WRITE，保存要发送的写命令参数
		//如果是EVENT，保存Event传回来的数据
		//如果是NOTIFY，表示发送参数
		//如果值为Null,表示该值无意义
		uint8_t* pStorage;
		int storageLen;

		//如果是READ，pData指向要发送的读命令参数
		//如果是WRITE，作为pStorage的Mirror变量，保存发送成功的数据，用于和pStorage比较是否发生变化，确定是否需要发送新的写命令
		//如果是EVENT，pData指向事件响应参数数据指针
		//如果是NOTIFY类型,pData=Null
		//如果值为Null,表示该值无意义
		uint8_t* pData;
		int dataLen;

		//const ModCmdFmt*  fmt;//帧格式定义
		ModEventFn Event;	//事件回调函数

		//如果是READ，作为pStorage的Mirror变量，pStorage同时用于READ和WRITE命令时有用
		//MirrorData的数据长度必须大于或者等于storageLen。
		//如果是WRITE，没用
		//如果是EVENT，没用
		//如果是NOTIFY，没用
		uint8_t* pMirrorData;

	}ModCmd;

	typedef struct _Mod
	{
		uint8_t addr;			//设备地址
		MOD_FSM state;

		uint16_t txBufLen;		//发送帧长度
		uint16_t rxBufLen;		//接收的的帧长度

		//记录接收数据帧的当前Ticks，用于计算接收一个数据帧内2个数据之间是否超时
		uint32_t rxDataTicks;

		/*****************************************************************/
		uint8_t reTxCount;	//重发次数
		uint8_t maxTxCount;	//最大重发次数

		uint32_t rxRspTicks;			//接收响应的Ticks，用于计算发送帧间隔

		uint32_t waitRspMs;			//等待响应时间
		SwTimer waitRspTimer;		//等待响应定时器

		const ModCmd* pWaitRspCmd;	//当前的执行的请求，正在等待RSP的命令，可能是READ/WRITE，Null表示没有

		/*****************************************************************/
		const ModCfg* cfg;
		const ModFrameCfg* frameCfg;
	}Mod;

	void Mod_Init(Mod* pMod, const ModCfg* cfg, const ModFrameCfg* frameCfg);
	void Mod_Run(Mod* pMod);

	/************************************
	函数功能：协议对象复位，
		取消已经发送正在等待响应的命令，
		取消所有等待发送的命令
		清除Rx缓冲区和Req缓冲区
	参数说明：
		pMod：Mod对象。
	返回值无
	***************************************/
	void Mod_Reset(Mod* pMod);
	void Mod_ResetCmds(const ModCfg* cfg);

	/************************************
	函数功能：从总线或者中断中接收数据，保存到接收缓冲区rxBufQueue队列中
	参数说明：
		pMod：Mod对象。
		pData：从总线或者中断中接收到的数据。
		len：数据长度。
	返回值无
	***************************************/
	void Mod_RxData(Mod* pMod, uint8_t* pData, int len);


	/*******************************************
	函数功能：发送命令。
	参数说明：
		pMod：Mod对象。
		cmd：发送的命令码。
	返回值：无
	*******************************************/
	void Mod_SendCmd(const ModCfg* cfg, uint8_t cmd);

	/*******************************************
	函数功能：延时发送命令。
	参数说明：
		pMod：Mod对象。
		cmd：发送的命令码。
		delayMs：延时发送事件，0表示不延时，立即发送
	返回值：无
	*******************************************/
	void Mod_DelaySendCmd(const ModCfg* cfg, uint8_t cmd, uint32_t delayMs);

	//传输协议对象是否空闲
	Bool Mod_isIdle(Mod* pMod);
	Bool Mod_SwitchCfg(Mod* pMod, const ModCfg* cfg);
	const ModCmd* Mod_FindCmdItem(const ModCmd* pCmd, int count, uint8_t cmd);

#ifdef __cplusplus
}
#endif

#endif


