
#include "Common.h"
#include "JT808.h"
#include "MsgDef.h"
#include "JtUtp.h"

static JT808 g_Jt;
JT808* g_pJt = &g_Jt;
static Utp g_JtUtp;
static uint32_t g_hbIntervalMs = 2000;	//MCU心跳时间间隔，单位Ms

void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2);

static JT808fsmFn JT808_findFsm(JT_state state);

UTP_EVENT_RC JT808_cmd_getSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_TX_START)
	{
	}
	return UTP_EVENT_RC_SUCCESS;
}

void JT808_switchState(JT808* pJt, JT_state newState)
{
	if (pJt->opState == newState) return;

	switch (newState)
	{
		case JT_STATE_INIT:
		{
			//发送心跳帧
			Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
			break;
		}
		case JT_STATE_SLEEP:
		{
			break;
		}
		case JT_STATE_WAKEUP:
		{
			break;
		}
		case JT_STATE_PREOPERATION:
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_ID);
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_CFG);

			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);
			break;
		}
		case JT_STATE_OPERATION:
		{
			break;
		}
	}

	pJt->fsm = JT808_findFsm(newState);
}

UTP_EVENT_RC JT808_event_simHb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_CHANGED_BEFORE)
	{
		JT_state newOp = *pCmd->pExt->transferData;
		JT808_switchState(pJt, newOp);
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_event_devStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}

//所有的命令，有传输事件发生，都会调用该回调函数
UTP_EVENT_RC JT808_utpEventCb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_REQ_SUCCESS)
	{
		JT808_fsm(MSG_UTP_REQ_DONE, (uint32_t)pCmd, ev);
	}

	if (pCmd->cmd == JTCMD_MCU_HB)
	{
		if (ev == UTP_REQ_SUCCESS || ev == UTP_REQ_FAILED)
		{
			Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

void JT808_fsm_init(uint8_t msgID, uint32_t param1, uint32_t param2)
{
}

void JT808_fsm_preoperation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_UTP_REQ_DONE)
	{
		//获取到所有的设备信息后，切换到OPERATION状态
		const UtpCmd* pCmd = (UtpCmd*)param1;
		if (pCmd->cmd == JTCMD_CMD_GET_FILE_INFO)
		{
			g_Jt.setToOpState = JT_STATE_OPERATION;
		}
	}
}

void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{

}

void JT808_fsm_sleep(uint8_t msgID, uint32_t param1, uint32_t param2)
{

}

void JT808_fsm_wakeup(uint8_t msgID, uint32_t param1, uint32_t param2)
{

}

static JT808fsmFn JT808_findFsm(JT_state state)
{
	struct
	{
		JT_state state;
		JT808fsmFn fsm;
	}
	static const fsmDispatch[] =
	{
		{JT_STATE_INIT			, JT808_fsm_init},
		{JT_STATE_SLEEP			, JT808_fsm_sleep},
		{JT_STATE_WAKEUP		, JT808_fsm_wakeup},
		{JT_STATE_PREOPERATION	, JT808_fsm_preoperation},
		{JT_STATE_OPERATION		, JT808_fsm_operation},
	};
	for (int i = 0; i < GET_ELEMENT_COUNT(fsmDispatch); i++)
	{
		if (g_Jt.opState == fsmDispatch[i].state)
		{
			return fsmDispatch[i].fsm;
		}
	}

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	g_pJt->fsm(msgID, param1, param2);
}

//发送数据到总线
int JT808_txData(const uint8_t* pData, int len)
{
	//transfer data to bus.
	return len;
}

//从总线上接收数据
void JT808_rxDataProc(const uint8_t* pData, int len)
{
	Utp_RxData(&g_JtUtp, pData, len);
}

void JT808_timerProc()
{
}

void JT808_run()
{
	JT808_timerProc();
	JT808_fsm(MSG_RUN, 0, 0);
	Utp_Run(&g_JtUtp);
}

Bool JT808_wakeup()
{
	return True;
}

Bool JT808_sleep()
{
	return True;
}

void JT808_start()
{
	//启动硬件，使能中断
	JT808_switchState(g_pJt, JT_STATE_INIT);
}

/************************************************
*以下变量是协议变量，仅供协议使用
************************************************/
void JT808_init()
{
	#define JT_CMD_SIZE 7
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	//static uint8_t g_JtState = JT_STATE_INIT;
	static uint16_t g_bleEnCtrl = 0;
	static uint8_t g_protocolVer = 1;	//传输协议版本号
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//位置越靠前，发送优先级越高
		{&g_JtCmdEx[0],UTP_NOTIFY, JTCMD_MCU_HB, "McuHb"		, (uint8_t*)& g_hbIntervalMs, 4},
		{&g_JtCmdEx[5],UTP_NOTIFY, JTCMD_SIM_HB, "SimHb"		, (uint8_t*)& g_Jt.opState, 1, Null, 0, (UtpEventFn)JT808_event_simHb},

		{&g_JtCmdEx[1],UTP_WRITE , JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.setToOpState, 1},

		{&g_JtCmdEx[2],UTP_READ , JTCMD_CMD_GET_SIM_ID, "GetSimID"	, (uint8_t*)& g_Jt.simID, sizeof(SimID), &g_protocolVer, 1},
		{&g_JtCmdEx[3],UTP_READ , JTCMD_CMD_GET_SIM_CFG, "GetSimCfg", Null, 0, Null, 0, (UtpEventFn)JT808_cmd_getSimCfg},
		{&g_JtCmdEx[4],UTP_WRITE, JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.bleEnCtrl, 2, (uint8_t*)&g_bleEnCtrl, 2},

		{&g_JtCmdEx[6],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},
	};

	static const UtpCfg g_cfg =
	{
		.cmdCount = JT_CMD_SIZE,
		.cmdArray = g_JtCmd,
		.TxFn = JT808_txData,
		.TresferEvent = (UtpEventFn)JT808_utpEventCb,
		.pCbObj = &g_Jt,
	};

	static const Obj obj = { "JT808", JT808_start, (ObjFn)JT808_sleep, JT808_run };
	ObjList_add(&obj);

	g_Jt.opState = JT_STATE_UNKNOWN;	//初始化为一个UNKNOWN值
	Utp_Init(&g_JtUtp, &g_cfg, &g_jtFrameCfg);
}
