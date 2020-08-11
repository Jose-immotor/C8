#ifndef __UTP_DEF_H_
#define __UTP_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

#define UTP_PROTOCOL_VER	1

#define UTP_NO_RESULT_BYTE_IN_RSP		0xFF	//无效的数据索引位

#define UTP_PURE_DATA_SIZE 128	//UTP纯数据大小
#define UTP_REQ_BUF_SIZE 	UTP_PURE_DATA_SIZE
#define UTP_RSP_BUF_SIZE 	UTP_PURE_DATA_SIZE

#define UTP_FRAME_HEAD 		0x7E	//0x7D 0x02
#define UTP_FRAME_TAIL 		0x7E	//0x7D 0x02

#define UTP_FRAME_ESCAPE 	0x7D	//0x7D 0x01
#define UTP_FRAME_ESCAPE_HREAD   0x02
#define UTP_FRAME_ESCAPE_TAIL    0x02
#define UTP_FRAME_ESCAPE_ESCAPE  0x01

#define UTP_WAIT_RSP_TIME_MS	1000	//发送请求等待响应时间
#define UTP_RETX_REQ_COUNT		3		//请求重发次数

//UTP 错误码定义
typedef enum _OP_CODE
{
	 OP_SUCCESS 		= 0		
	,OP_PARAM_INVALID	= 1 //该参数无效
	,OP_UNSUPPORTED		= 2 //该命令不支持
	,OP_CRC_ERROR		= 3	//校验错误
	,OP_NOT_READY		= 4	//设备没准备好
	,OP_USERID_ERROR	= 5	//USER_ID错误
	,OP_HW_ERROR		= 6	//执行失败
	,OP_18650BAT_V_OVER_LOW	= 7	//18650电压太低
	,OP_NOT_ALLOW_DISCHARGE	= 8	//不支持放电
	
	,OP_PENDING			= 0xF0	//执行等待
	,OP_NO_RSP			= 0xF1	//No response
	,OP_FAILED			= 0xFF	//执行失败
}OP_CODE;

/***************
传输事件定义
***************/
typedef enum
{
	UTP_TX_START,		//发送命令开始, 可在该事件中配置通信参数：m_waitRspMs, m_maxTxCount

	UTP_CHANGED_BEFORE,	//pStorage值改变之前
	UTP_CHANGED_AFTER,	//pStorage值改变之后
	UTP_GET_RSP,		//接收到请求,获取响应，响应数据在transferData, transferLen中

	UTP_REQ_FAILED,		//请求结束，失败，原因：可能传输超时，或者响应返回失败码。
	UTP_REQ_SUCCESS,	//请求结束，成功，传输成功并且响应返回成功码
}UTP_TXF_EVENT;

typedef enum
{
	UTP_EVENT_RC_SUCCESS,	//事件处理成功
	UTP_EVENT_RC_FAILED,	//事件处理失败
	UTP_EVENT_RC_DO_NOTHING,	//事件没有处理
}UTP_EVENT_RC;

//UTP 传输状态
typedef enum
{
	  UTP_FSM_INIT = 0	//初始化
	, UTP_FSM_RX_REQ	//接收到请求
	, UTP_FSM_WAIT_RSP	//等待响应
}UTP_FSM;

typedef enum
{
	UTP_READ = 0	//读命令类型，需要应答
	, UTP_WRITE		//写命令类型，需要应答
	, UTP_EVENT		//事件类型  ，需要应答
	, UTP_NOTIFY	//通知类型  ，不需要应答
}UTP_CMD_TYPE;

typedef enum _UTP_RCV_RSP_RC
{
	RSP_SUCCESS = 0
	, RSP_TIMEOUT
	, RSP_FAILED
	, RSP_CANCEL		//取消发送
}UTP_RCV_RSP_RC;

struct _UtpCmd;
typedef UTP_EVENT_RC(*UtpCmdFn)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);

typedef struct _UtpCmdEx
{
	/*
	如果是EVENT，
		在UTP_GET_RSP事件之前transferData指向接收到的数据
		在UTP_GET_RSP事件中transferData默认指向UtpCmd->pData，也可以
			在事件函数中修改，指向要应答的数据指针，由上层程序决定
	如果是READ/WRITE,指向应答数据
	*/
	uint8_t* transferData;	//传输数据指针
	uint8_t transferLen;		//传输数据长度

	/*****************
	发送请求或接收响应时的时间戳Ticks.
	对于UTP_READ/UTP_WRITE类型：保存接收响应时的Ticks：
	对于UTP_EVENT类型：无效
	对于UTP_NOTIFY类型：发送请求时的Ticks。
	******************/
	uint32_t rxRspTicks;	

	/*****************
	发送请求延时时间，对下列命令类型有效：
		0：表示不发送，
		其他值：表示延时指定的时间MS发送
	******************/
	uint32_t sendDelayMs;

	UTP_RCV_RSP_RC rcvRspErr;	//接收响应错误码
}UtpCmdEx;

typedef struct _UtpCmd
{
	UtpCmdEx* pExt;	//UtpCmd扩展
	
	UTP_CMD_TYPE type;	//读或者写命令，参考UTP_CMD_TYPE定义
	uint8_t cmd;			//命令码
	const char* cmdName;//命令名称

	//如果是READ，保存读回来的数据
	//如果是WRITE，保存要发送的写命令参数
	//如果是EVENT，保存Event传回来的数据
	//如果值为Null,表示该值无意义
	uint8_t* pStorage;
	int storageLen;	

	//如果是READ，pData指向要发送的读命令参数
	//如果是WRITE，pData指向已经发送成功的数据，用于和pStorage比较是否有变化，确定是否需要发送新的写命令
	//如果是EVENT，pData指向事件响应参数数据指针
	//如果值为Null,表示该值无意义
	uint8_t* pData;	
	int dataLen;	

	UtpCmdFn Event;	//事件回调函数
}UtpCmd;

typedef OP_CODE(*UtpDispatchFn)(const uint8_t* pData, uint8_t dataLen, uint8_t* rspData, uint8_t* pRspLen);
typedef struct _RxReqDispatch
{
	uint8_t cmd;
	UtpDispatchFn Proc;
}RxReqDispatch;

#ifdef __cplusplus
}
#endif

#endif



