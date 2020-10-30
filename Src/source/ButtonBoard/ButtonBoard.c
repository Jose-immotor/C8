
#include "Common.h"
#include "ButtonBoard.h"
#include "JtUtp.h"
#include "drv_can.h"

Butt g_Butt;
static Utp g_ButUtp;
uint8_t g_buhb[4] = {0x00, 0x00, 0x07, 0xD0};

void Butt_fsm_init(uint8_t msgID, uint32_t param1, uint32_t param2)
{
}

void Butt_fsm_preoperation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
//	if (msgID == MSG_UTP_REQ_DONE)
//	{
//		//获取到所有的设备信息后，切换到OPERATION状态
//		const UtpCmd* pCmd = (UtpCmd*)param1;
//		if (pCmd->cmd == JTCMD_CMD_GET_FILE_INFO)
//		{
//			g_Jt.setToOpState = JT_STATE_OPERATION;
//			Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
//		}
//	}
}

void Butt_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
//	if (msgID == MSG_RUN)
//	{
//		// update
//		if (Utp_isIdle(&g_JtUtp) && (g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT) )	// 空闲 & 连接网络s
//		{
//			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
//			if (len) 
//			{
//				g_txlen = len;
//				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
//			}
//		}
//	}
}

void Butt_fsm_sleep(uint8_t msgID, uint32_t param1, uint32_t param2)
{

}

void Butt_fsm_wakeup(uint8_t msgID, uint32_t param1, uint32_t param2)
{

}

static ButtfsmFn Butt_findFsm(BU_state state)
{
	struct
	{
		BU_state state;
		ButtfsmFn fsm;
	}
	static const fsmDispatch[] =
	{
		{BU_STATE_INIT			, Butt_fsm_init},
		{BU_STATE_SLEEP			, Butt_fsm_sleep},
		{BU_STATE_WAKEUP		, Butt_fsm_wakeup},
		{BU_STATE_PREOPERATION	, Butt_fsm_preoperation},
		{BU_STATE_OPERATION		, Butt_fsm_operation},
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

void Butt_fsm(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	g_Butt.fsm(msgID, param1, param2);
}

void BU_switchState(Butt* pJt, BU_state newState)
{
	if (pJt->opState == newState) return;

	switch (newState)
	{
		case BU_STATE_INIT:
		{
			//发送心跳帧
			Utp_SendCmd(&g_ButUtp, BUCMD_MCU_HB);
			break;
		}
		case BU_STATE_SLEEP:
		{
			break;
		}
		case BU_STATE_WAKEUP:
		{
			break;
		}
		case BU_STATE_PREOPERATION:
		{
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);// 0x11
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_ID); //0x30
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_CFG); //0x31
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN); // 0x33			
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);//0x16
			break;
		}
		case BU_STATE_OPERATION:
		{
//			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
			break;
		}
	}

	pJt->fsm = Butt_findFsm(newState);
}

//所有的命令，有传输事件发生，都会调用该回调函数
UTP_EVENT_RC Butt_utpEventCb(Butt* pBu, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_REQ_SUCCESS)
	{
		Butt_fsm(MSG_UTP_REQ_DONE, (uint32_t)pCmd, ev);
	}

	if (pCmd->cmd == BUCMD_MCU_HB)
	{
		if (ev == UTP_REQ_SUCCESS)
		{
			Utp_DelaySendCmd(&g_ButUtp, BUCMD_MCU_HB, 2000);
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

extern can_trasnmit_message_struct transmit_message;
#define	BUCAN_TX_TIMEOUT_MS		(30)
int Butt_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmd为CAN协议的PF
{
	uint32_t send_len = 0;
	uint32_t tx_timeout = 0;
	//transfer data to bus.
	while(len)
	{
		if(len > 8)
		{
			send_len = 8;
		}
		else
		{
			send_len = len;
		}
		memcpy(transmit_message.tx_data, pData, send_len);	
		transmit_message.tx_dlen = send_len;
		transmit_message.tx_efid = 0x00003010;
		can_message_transmit(CAN1, &transmit_message);
		PFL(DL_CAN, "CANTx:");
		PFL(DL_CAN, "%08x ",transmit_message.tx_efid);
		DUMP_BYTE_LEVEL(DL_CAN,&transmit_message.tx_data,transmit_message.tx_dlen);
		PFL(DL_CAN, "\n");
//		rt_thread_mdelay(20);
		tx_timeout = GET_TICKS();
		while(
			CAN_TRANSMIT_PENDING == can_transmit_states(CAN1, CAN_MAILBOX0) &&
			GET_TICKS() - tx_timeout <BUCAN_TX_TIMEOUT_MS );

		if(len > 8)
		{
			pData = pData + 8;
			len = len - 8;
		}
		else
		{
			break;
		}
	}

	return len;
}

//xx 从总线上接收数据
void Butt_rxDataProc(const uint8_t* pData, int len)
{
	Utp_RxData(&g_ButUtp, pData, len);
}

void Button_start()
{
	BU_switchState(&g_Butt, BU_STATE_INIT);
}

void Button_sleep()
{
	
}

void Button_run()
{
	Butt_fsm(MSG_RUN, 0, 0);
	Utp_Run(&g_ButUtp);
}

void Button_init()
{
	#define BU_CMD_SIZE 2
	static UtpCmdEx g_BuCmdEx[BU_CMD_SIZE];
	static const UtpCmd g_BuCmd[BU_CMD_SIZE] =
	{
		//位置越靠前，发送优先级越高
		{&g_BuCmdEx[0],UTP_NOTIFY		, BUCMD_MCU_HB, "McuHb", g_buhb, sizeof(g_buhb),
			Null,0,Null},	/*(uint8_t*)& g_hbIntervalMs*/
//		{&g_BuCmdEx[1],UTP_EVENT_NOTIFY	, BUCMD_SIM_HB, "ButtHb",(uint8_t*)& g_Butt.opState, 1,
//			Null, 0, (UtpEventFn)JT808_event_simHb},
	};
	static const UtpCfg g_cfg =
	{
		.cmdCount = BU_CMD_SIZE,
		.cmdArray = g_BuCmd,
		.TxFn = Butt_txData,
		.TresferEvent = (UtpEventFn)Butt_utpEventCb,
		.pCbObj = &g_Butt,
	};
	
	static const Obj obj = { "BUTTON", Button_start, Button_sleep, Button_run };
	ObjList_add(&obj);
	
	Utp_Init(&g_ButUtp, &g_cfg, &g_jtFrameCfg);
	g_Butt.opState = BU_STATE_UNKNOWN;
	g_Butt.online_sta = 0;//不在线
}
