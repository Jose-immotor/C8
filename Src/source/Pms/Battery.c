
#include "Common.h"
#include "Battery.h"
#include "Modbus.h"
#include "Pms.h"

extern Mod* g_pModBus;
static void Bat_fsm(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2);

void Bat_dump(const Battery* pBat)
{
	Printf("Battery[%d]:\n", pBat->port);
	Printf("\t presentStatus(1=NOTIN;2-IN)=%d.\n", pBat->presentStatus);
	Printf("\t opStatus=%d.\n", pBat->opStatus);
}

void Bat_bmsInfoDump(const Battery* pBat)
{
	const uint8_t* pByte = (uint8_t*) & (pBat->bmsID.sn34);
	Printf("\tPort[%d][%02X%02X%02X%02X%02X%02X]:\n"
		, pBat->port, pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5]);
	Printf("\t\tsoc(0.1)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.soc)));
	Printf("\t\tvolt(0.01V)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.tvolt)));
	Printf("\t\tcurr(0.01A)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr)));
	Printf("\t\tstate=0x%04X\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.state)));
}

const uint8* Bat_getBID(Battery* pBat)
{
	return (uint8*)&pBat->bmsID.sn34;
}

static void Bat_sendCmd(Battery* pBat, uint8_t cmd)
{
	Mod_SendCmd(pBat->cfg, cmd);
}

void Bat_msg(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	Bat_fsm(pBat, msgId, param1, param2);
}

static void Bat_onPlugOut(Battery* pBat)
{
	pBat->presentStatus = BAT_NOT_IN;
	g_pdoInfo.isBat0In =0;
	PFL(DL_PMS,"Battery out!\n");
	LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_BATOUT,0);
	NVC_PLAY(NVC_BAT_PLUG_OUT);
//	if (pBat->isActive)
//	{
//		Mod_Reset(g_pModBus);
//	}
//	else
	{
		Mod_ResetCmds(pBat->cfg);
	}

	memset(&pBat->bmsCtrl, 0, sizeof(BmsReg_ctrl));
	memset(&pBat->bmsInfo, 0, sizeof(BmsReg_deviceInfo));
	memset(&pBat->bmsID, 0, sizeof(BmsReg_info));
}

static void Bat_switchStatus(Battery* pBat, BmsOpStatus opStatus)
{
	if (opStatus == pBat->opStatus) return;

	pBat->idleTicks = 0;
	pBat->statusSwitchTicks = GET_TICKS();
	if (BmsStatus_init == opStatus)
	{
		Mod_ResetCmds(pBat->cfg);
	}
	else if (BmsStatus_readInfo == opStatus)
	{
//		pBat->presentStatus = BAT_IN;
		Bat_sendCmd(pBat, BMS_READ_INFO_1);
		Bat_sendCmd(pBat, BMS_READ_INFO_2);
	}
	else if (BmsStatus_idle == opStatus)
	{
		pBat->idleTicks = GET_TICKS();
	}
	else if (BmsStatus_sleep == opStatus)
	{
	}
	else if (BmsStatus_deepSleep == opStatus)
	{
	}

	pBat->opStatus = opStatus;
}

//所有的BMS所有命令公共的事件回调函数
MOD_EVENT_RC Bat_event(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		PFL(DL_NFC,"modbus cmd[%d] req success!\n",pCmd->cmd);
		Bat_fsm(pBat, BmsMsg_cmdDone, pCmd->cmd, ev);
		if (BMS_WRITE_CTRL == pCmd->cmd)
		{
			BmsRegCtrl* pCtrl = (BmsRegCtrl*)&pBat->bmsCtrl.ctrl;
			if (pCtrl->sleepEn)
			{
				Bat_switchStatus(pBat, BmsStatus_sleep);
			}
			else if (pCtrl->deepSleepEn)
			{
				Bat_switchStatus(pBat, BmsStatus_deepSleep);
			}
			else
			{
				//设置完CTRL寄存器必须重新读取BMS_CTRL
				Bat_sendCmd(pBat, BMS_READ_CTRL);
			}
		}
	}
	else if (ev == MOD_REQ_FAILED)
	{
		PFL(DL_NFC,"modbus cmd[%d] req failed!rsp code:%d\n",pCmd->cmd,pCmd->pExt->rcvRspErr);
		
//		if (BMS_READ_ID == pCmd->cmd)
//		{
//			if (pCmd->pExt->rcvRspErr == MOD_RSP_TIMEOUT)
//			{
				Bat_fsm(pBat, BmsMsg_batPlugout, 0, 0);
//			}
//		}
	}
	else if (ev == MOD_TX_START)
	{
		PFL(DL_NFC,"modbus cmd[%d] start!\n",pCmd->cmd);
	}
	return MOD_EVENT_RC_SUCCESS;
}

//所有的BMS单个命令的事件回调函数
MOD_EVENT_RC Bat_event_readBmsInfo(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		if(pBat->presentStatus != BAT_IN)
		{
			PFL(DL_PMS,"Battery in!\n");
			g_pdoInfo.isBat0In =1;
			LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_BATIN,
								bigendian16_get((uint8*)&pBat->bmsInfo.soc));
			NVC_PLAY(NVC_BAT_PLUG_IN);
		}			
		pBat->presentStatus = BAT_IN;
	}
	if (ev == MOD_CHANGED_BEFORE)
	{

	}

	return MOD_EVENT_RC_SUCCESS;
}


static void Bat_fsm_init(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == BmsMsg_cmdDone)
	{
		Bat_sendCmd(pBat, BMS_READ_CTRL);
		Bat_switchStatus(pBat, BmsStatus_readInfo);
	}
	else if (msgId == BmsMsg_active)
	{
		Bat_sendCmd(pBat, BMS_READ_ID);
	}
}

static void Bat_fsm_readInfo(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == BmsMsg_active)
	{
		Bat_sendCmd(pBat, BMS_READ_ID);
		Bat_sendCmd(pBat, BMS_READ_INFO_1);
		Bat_sendCmd(pBat, BMS_READ_INFO_2);
	}
	else if (msgId == BmsMsg_batPlugout)
	{
		Bat_onPlugOut(pBat);
		Bat_switchStatus(pBat, BmsStatus_init);
	}
}

static void Bat_fsm(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	struct
	{
		BmsOpStatus opStatus;
		void (*OpFun)(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2);
	}
	const static g_fsms[] =
	{
		{BmsStatus_init			, Bat_fsm_init},
		{BmsStatus_readInfo		, Bat_fsm_readInfo},
		//{BmsStatus_idle		, Bat_fsm_idle},
		//{BmsStatus_sleep		, Bat_fsm_sleep},
		//{BmsStatus_deepSleep	, Bat_fsm_deepSleep},
	 };

	if (msgId == BmsMsg_deactive)
	{
		pBat->isActive = False;
	}
	else if (msgId == BmsMsg_sleep)
	{
		pBat->pBmsReg_Ctrl->sleepEn = 1;
		Bat_sendCmd(pBat, BMS_WRITE_CTRL);
	}
	else if (msgId == BmsMsg_deepSleep)
	{
		pBat->pBmsReg_Ctrl->sleepEn = 0;
		pBat->pBmsReg_Ctrl->deepSleepEn = 1;
		Bat_sendCmd(pBat, BMS_WRITE_CTRL);
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_fsms); i++)
	{
		if (g_fsms[i].opStatus == pBat->opStatus)
		{
			g_fsms[i].OpFun(pBat, msgId, param1, param2);
			break;
		}
	}
}

//电池信息有效
Bool Bat_isReady(Battery* pBat)
{
	return pBat->presentStatus == BAT_IN;
}

void Bat_setPreDischg(Battery* pBat, Bool en){ pBat->pBmsReg_Ctrl->preDischg = en  ; Bat_sendCmd(pBat, BMS_READ_CTRL); }
void Bat_setDischg(Battery* pBat, Bool en)	 { pBat->pBmsReg_Ctrl->dischgEn = en   ; Bat_sendCmd(pBat, BMS_READ_CTRL); }
void Bat_setChg(Battery* pBat, Bool en)		 { pBat->pBmsReg_Ctrl->chgEn = en      ; Bat_sendCmd(pBat, BMS_READ_CTRL); }
void Bat_setSleep(Battery* pBat, Bool en)	 { pBat->pBmsReg_Ctrl->sleepEn = en    ; Bat_sendCmd(pBat, BMS_READ_CTRL); }
void Bat_setDeepSleep(Battery* pBat, Bool en){ pBat->pBmsReg_Ctrl->deepSleepEn = en; Bat_sendCmd(pBat, BMS_READ_CTRL); }

void Bat_run(Battery* pBat)
{
	Bat_fsm(pBat, BmsMsg_run, 0, 0);
}

void Bat_start(Battery* pBat)
{
}

void Bat_init(Battery* pBat, int port, const ModCfg* cfg)
{
	memset(pBat, 0, sizeof(Battery));

	pBat->port = port;
	pBat->presentStatus = BAT_UNKNOWN;
	pBat->opStatus = BmsStatus_init;
	pBat->cfg = cfg;
	pBat->pBmsReg_Ctrl = (BmsRegCtrl*)& pBat->writeBmsCtrl;

}
