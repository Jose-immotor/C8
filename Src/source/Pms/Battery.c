
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
	Printf("\t\tstate=0x%04X\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.state)));
	Printf("\t\tsoc(0.1)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.soc)));
	Printf("\t\tvolt(0.01V)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.tvolt)));
	Printf("\t\tcurr(0.01A)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr)));
	Printf("\t\ttemp(0.1��)=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.htemp)));
	Printf("\t\tcycle=%d\r\n", bigendian16_get((uint8*)(&pBat->bmsInfo.cycle)));
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

//���е�BMS������������¼��ص�����
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
				//������CTRL�Ĵ����������¶�ȡBMS_CTRL
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


MOD_EVENT_RC Bat_event_readBmsID(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		g_Ble.portState.property[0].nominalVol = bigendian16_get((uint8*)(&pBat->bmsID.bvolt));
		g_Ble.portState.property[0].nominalCap = bigendian16_get((uint8*)(&pBat->bmsID.bcap));
		g_Ble.batDesc[0].serialNum[0] = pBat->bmsID.sn34;
		g_Ble.batDesc[0].serialNum[1] = pBat->bmsID.sn34>>8;
		g_Ble.batDesc[0].serialNum[2] = pBat->bmsID.sn56;
		g_Ble.batDesc[0].serialNum[3] = pBat->bmsID.sn56>>8;
		g_Ble.batDesc[0].serialNum[4] = pBat->bmsID.sn78;
		g_Ble.batDesc[0].serialNum[5] = pBat->bmsID.sn78>>8;
		g_Ble.batDesc[0].damage = bigendian16_get((uint8*)(&pBat->bmsID.ltsta));

	}
	if (ev == MOD_CHANGED_BEFORE)
	{

	}

	return MOD_EVENT_RC_SUCCESS;
}

//���е�BMS����������¼��ص�����
MOD_EVENT_RC Bat_event_readBmsInfo(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		uint16_t tmp_value;
		
		if(pBat->presentStatus != BAT_IN)
		{
			PFL(DL_PMS,"Battery in!\n");
			g_pdoInfo.isBat0In =1;
			LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_BATIN,
								bigendian16_get((uint8*)&pBat->bmsInfo.soc));
			NVC_PLAY(NVC_BAT_PLUG_IN);
		}			
		pBat->presentStatus = BAT_IN;
		g_Ble.portState.property[0].nominalCur = bigendian16_get((uint8*)(&pBat->bmsInfo.dsop));
		
		tmp_value = bigendian16_get((uint8*)(&pBat->bmsInfo.soc));
		if(tmp_value == 0xFFFF)
        {
            g_Ble.batDesc[0].SOC = 0xFF;
        }
        else
        {
            g_Ble.batDesc[0].SOC = tmp_value/10;
        }
		g_Ble.batDesc[0].voltage = bigendian16_get((uint8*)(&pBat->bmsInfo.tvolt));
		g_Ble.batDesc[0].current = bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr));
		
		tmp_value = bigendian16_get((uint8*)(&pBat->bmsInfo.htemp));
		if(tmp_value == 0xFFFF)
        {
            g_Ble.batDesc[0].temp = 0xFF;
        }
        else
        {
            g_Ble.batDesc[0].temp = tmp_value/10;
        }
		tmp_value = bigendian16_get((uint8*)(&pBat->bmsInfo.opft1));
        g_Ble.batDesc[0].fault = 0;
        if(tmp_value&(1<<0))	g_Ble.batDesc[0].fault |= (1<<0);//��ѹ
        if(tmp_value&((1<<1)|(1<<2)))	g_Ble.batDesc[0].fault |= (1<<1);//Ƿѹ
        if(tmp_value&((1<<3)|((1<<4)|(1<<5))))	g_Ble.batDesc[0].fault |= (1<<2);//����
        if(tmp_value &((1<<6)|(1<<7)|(1<<10)|(1<<11)|(1<<12)))	g_Ble.batDesc[0].fault |= (1<<3);//����
        if(tmp_value &((1<<8)|(1<<9)))	g_Ble.batDesc[0].fault |= (1<<4);//����
        tmp_value = bigendian16_get((uint8*)(&pBat->bmsInfo.devft1));
        if((tmp_value &((1<<14)|(1<<15)))||
            (pBat->bmsInfo.devft1 != 0)||(pBat->bmsInfo.devft2 != 0)||
            (pBat->bmsInfo.opft1 != 0)|| (pBat->bmsInfo.opft2 != 0)||
            (pBat->bmsInfo.opwarn1 != 0)||(pBat->bmsInfo.opwarn2 != 0))
        {
            //����
            g_Ble.batDesc[0].fault |= (1<<5);
        }
		g_Ble.batDesc[0].cycleCount = bigendian16_get((uint8*)(&pBat->bmsInfo.cycle));
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

//�����Ϣ��Ч
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
