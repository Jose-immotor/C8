
#include "Common.h"
#include "JT808.h"
#include "MsgDef.h"
#include "JtUtp.h"
#include "JtTlv0900.h"
#include "JtTlv8900.h"
#include "JtTlv8103.h"
#include "Ble.h"
#include "drv_can.h"
#include "string.h"

JT808 g_Jt;
JT808* g_pJt = &g_Jt;
static Utp g_JtUtp;
static uint32_t g_hbIntervalMs = 2000;	//MCU����ʱ��������λMs
static uint8_t g_txBuf[100];
static uint8_t g_txlen = 0;
uint8_t g_hbdata[4] = {0x00, 0x00, 0x07, 0xD0};

void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2);

static JT808fsmFn JT808_findFsm(JT_state state);

UTP_EVENT_RC JT808_cmd_getSimID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevProperty) - JT_DEV_HW_VER_SIZE - JT_DEV_FW_VER_SIZE;
	if (ev == UTP_REQ_SUCCESS)
	{
		uint8 ver[JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE];

		//���յ����ݺ�property����Ľṹ��ƥ�䣬��Ҫ���¸�ֵHwVer��FwVer

		memcpy(ver, &g_Jt.property.hwVer, pCmd->pExt->transferLen - headSize - 1);
		memset(&g_Jt.property.hwVer, 0, JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE);

		if (ver[0] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(&g_Jt.property.hwVer, &ver[1], ver[0]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 hwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(&g_Jt.property.fwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 fwVer size(%d) error.\n", ver[offset]);
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	static int readParamOffset = 0;
	static int i = 0;
	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;

	if (ev == UTP_TX_START)
	{
		const TlvIn* p = &g_jtTlvInMgr_8103.itemArray[readParamOffset];
		//���ö�ȡ����
		for (i = 0; (i + readParamOffset) < g_jtTlvInMgr_8103.itemCount && i < 5; i++, p++)
		{
			paramIDs[i] = p->tag;
		}
		pCmd->pExt->transferLen = i * 2;
	}
	else if (ev == UTP_REQ_SUCCESS)	//��ȡ�ɹ�
	{
		readParamOffset += i;
		JtTlv8103_updateStorage(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		if (readParamOffset < g_jtTlvInMgr_8103.itemCount)
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
		}
		else //ȫ����ȡ���
		{
			i = 0;
			readParamOffset = 0;
		}
	}
	else if (ev == UTP_REQ_FAILED)	//��ȡʧ�ܣ����¶�ȡ
	{
		PFL_WARNING("Read sim param failed.\n");
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
			//��������֡
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

UTP_EVENT_RC JT808_event_rcvSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_CHANGED_AFTER)
	{
		JtTlv8900_proc(pCmd->pExt->transferData, pCmd->pExt->transferLen);
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_event_sendSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	static int readParamOffset = 0;
	static int i = 0;
	int len = 0;
	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;

	if(ev == UTP_TX_START)	
	{
		pCmd->pExt->transferLen = g_txlen;
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		g_txlen = 0;
	}

	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_event_rcvBleData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		uint8 rspLen = 0;
		pCmd->pExt->transferData = Ble_ReqProc(pCmd->pStorage, pCmd->storageLen, &rspLen);
		pCmd->pExt->transferLen = rspLen;
		if (pCmd->pExt->transferData == Null)
		{
			return UTP_EVENT_RC_FAILED;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//���е�����д����¼�������������øûص�����
UTP_EVENT_RC JT808_utpEventCb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_REQ_SUCCESS)
	{
		JT808_fsm(MSG_UTP_REQ_DONE, (uint32_t)pCmd, ev);
	}

	if (pCmd->cmd == JTCMD_MCU_HB)
	{
		if (ev == UTP_REQ_SUCCESS)
		{
			Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);
		}
	}
	else if (pCmd->cmd == JTCMD_CMD_SEND_TO_SVR)
	{
		if (ev == UTP_REQ_SUCCESS)
		{
			JtTlv0900_updateMirror(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
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
		//��ȡ�����е��豸��Ϣ���л���OPERATION״̬
		const UtpCmd* pCmd = (UtpCmd*)param1;
		if (pCmd->cmd == JTCMD_CMD_GET_FILE_INFO)
		{
			g_Jt.setToOpState = JT_STATE_OPERATION;
			Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
		}
	}
}

void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_RUN)
	{
		if (Utp_isIdle(&g_JtUtp))
		{
			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
			if (len) 
			{
				g_txlen = len;
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
			}
		}
	}
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
		if (state == fsmDispatch[i].state)
		{
			return fsmDispatch[i].fsm;
		}
	}

	//���򲻿������е�����
	Assert(False);
	return Null;
}

void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	g_pJt->fsm(msgID, param1, param2);
}

void JTcmd(int argc, char** argv)
{
 int  cmdCode = 0;
 sscanf(&(*argv[1]), "%d", &cmdCode);
	switch(cmdCode)
	{
		case JTCMD_SIM_HB: printf("receive Utpcmd1!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, cmdCode);break;
		case JTCMD_MCU_HB: printf("receive Utpcmd2!\r\n");rt_thread_mdelay(1);	Utp_SendCmd(&g_JtUtp, cmdCode);break;
		case JTCMD_SET_OP_STATE: printf("receive Utpcmd3!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, cmdCode);break;

		case 11: printf("receive Utpcmd11!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, 0x11);break;
		case 12: printf("receive Utpcmd12!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, 0x12);break;
		//case JTCMD_SET_OP_STATE: printf("receive Utpcmd3!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, cmdCode);break;
		case 80: printf("receive Utpcmd80!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, 0x80);break;
		case 160: printf("receive UtpcmdA0!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, 0xA0);break;
		case 15: printf("receive Utpcmd15!\r\n");rt_thread_mdelay(1); Utp_SendCmd(&g_JtUtp, 0x15);break;
	}
 
}
MSH_CMD_EXPORT(JTcmd, JT808_sendCmd<uint8_t ind>);

extern can_trasnmit_message_struct transmit_message;
//xx �������ݵ����� һ����෢��8���ֽ� ����ʱ���ж��Ƿ����һ��
int JT808_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmdΪCANЭ���PF
{
	uint32_t send_len = 0;
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
		can_message_transmit(CAN0, &transmit_message);
//		rt_thread_mdelay(20);
		while(CAN_TRANSMIT_PENDING == can_transmit_states(CAN0, CAN_MAILBOX0));

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


//xx �������Ͻ�������
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
	//����Ӳ����ʹ���ж�
	JT808_switchState(g_pJt, JT_STATE_INIT);
}

/************************************************
*���±�����Э�����������Э��ʹ��
************************************************/
void JT808_init()
{
	#define JT_CMD_SIZE 11
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	//static uint8_t g_JtState = JT_STATE_INIT;
	static uint16_t g_bleEnCtrl = 0;
	static uint8_t g_protocolVer = 1;	//����Э��汾��
	static uint8_t g_rxBuf[128];
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//λ��Խ��ǰ���������ȼ�Խ��
		{&g_JtCmdEx[0],UTP_NOTIFY, JTCMD_MCU_HB, "McuHb", g_hbdata, 4},	/*(uint8_t*)& g_hbIntervalMs*/
		{&g_JtCmdEx[1],UTP_EVENT_NOTIFY, JTCMD_SIM_HB, "SimHb", (uint8_t*)& g_Jt.opState, 1, Null, 0, (UtpEventFn)JT808_event_simHb},

		{&g_JtCmdEx[2],UTP_READ , JTCMD_CMD_GET_SIM_ID , "GetSimID"	, (uint8_t*)& g_Jt.property, sizeof(JtDevProperty), &g_protocolVer, 1, (UtpEventFn)JT808_cmd_getSimID},
		{&g_JtCmdEx[3],UTP_READ , JTCMD_CMD_GET_SIM_CFG, "GetSimCfg", (uint8_t*)& g_rxBuf, sizeof(g_rxBuf), (uint8_t*)& g_txBuf, sizeof(g_txBuf), (UtpEventFn)JT808_cmd_getSimCfg},

		{&g_JtCmdEx[4],UTP_WRITE , JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.setToOpState, 1},

		{&g_JtCmdEx[5],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},

		{&g_JtCmdEx[6],UTP_EVENT, JTCMD_EVT_RCV_SVR_DATA, "RcvSvrData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvSvrData},
		{&g_JtCmdEx[7],UTP_WRITE, JTCMD_CMD_SEND_TO_SVR, "SendDataToSvr", (uint8_t*)g_txBuf, sizeof(g_txBuf), Null, 0, (UtpEventFn)JT808_event_sendSvrData},

		{&g_JtCmdEx[8],UTP_WRITE, JTCMD_CMD_SEND_TO_SVR, "SendDataToSvr", (uint8_t*)g_txBuf, sizeof(g_txBuf)},

		{&g_JtCmdEx[9],UTP_EVENT, JTCMD_BLE_EVT_CNT, "BleCnt", (uint8_t*)g_rxBuf, sizeof(g_rxBuf)},
		{&g_JtCmdEx[9],UTP_EVENT, JTCMD_BLE_RCV_DAT, "BleRcvDat", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvBleData},
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

	g_Jt.opState = JT_STATE_UNKNOWN;	//��ʼ��Ϊһ��UNKNOWNֵ
	Utp_Init(&g_JtUtp, &g_cfg, &g_jtFrameCfg);

	JtTlv8900_init();
	JtTlv0900_init();
	JtTlv8103_init();
}
