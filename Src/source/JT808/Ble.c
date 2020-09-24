
/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 * 
 * Ble对象实现文件
 */

#include "Common.h"
#include "Ble.h"
#include "BleTpu.h"
#include "TlvIn.h"

Ble g_Ble;
static BleTpu g_BleTpu;

UTP_EVENT_RC Ble_getSelfTestResult(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		SelfTestResult*  pResult = (SelfTestResult*)pCmd->pExt->transferData;
		//pResult的结构赋值
		pResult->simState = 0;	//待定。
		//...
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC Ble_authent(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		if (BLE_USER_INVALID == BleUser_login(&pBle->user, (char*)pCmd->pStorage))
		{
			return ERR_USERID_INVALID;
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC Ble_setNvds(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		//const static TlvIn tlvs[] =
		//{
		//	{"SmtFwVer", 0x01, 7, Null, DT_STRUCT},
		//	{"PmsFwVer", 0x02, 7, Null, DT_STRUCT},
		//	{"SmtHwVer", 0x04, 2, Null, DT_STRUCT},
		//	{"SmtHwVer", 0x10, 1, Null, DT_STRUCT},
		//};

		//for (int i = 0; i < GET_ELEMENT_COUNT(tlvs); i++)
		//{
		//	if (tlvs[i].tag == pCmd->pStorage[0])
		//	{
		//		if (tlvs[i].storage)
		//			memcpy(tlvs[i].storage, pCmd->pStorage[2], MIN(pCmd->pStorage[1], tlvs[i].len));
		//	}
		//}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//检验命令是否允许执行
static Bool Ble_cmdIsAllow(Ble* pBle, uint8 cmd)
{
	//定义需要用户身份认证的命令码
	struct
	{
		BLE_USER_ROLE role;
		uint8 cmd;
	}
	static const cmdRoleMatch[] = 
	{
		{BLE_USER_ADMIN, REQ_ID_SET_NVDS},
		{BLE_USER_ADMIN, REQ_ID_ACTIVE_REQ},
		{BLE_USER_ADMIN, REQ_ID_BAT_VERIFY},
		{BLE_USER_ADMIN, REQ_ID_SET_ALARM_MODE},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(cmdRoleMatch); i++)
	{
		if (cmdRoleMatch[i].cmd == cmd)
		{
			return cmdRoleMatch[i].role & pBle->user.role;
		}
	}

	//默认允许
	return True;
}

UTP_EVENT_RC Ble_utpEventCb(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		//判断命令是否允许
		if (!Ble_cmdIsAllow(pBle, pCmd->cmd)) return ERR_CMD_NOT_ALLOW;

		if (pCmd->cmd == REQ_ID_GET_PORT_STATE)
		{
			uint8 port = *pCmd->pStorage;
			if (port >= MAX_BAT_COUNT) return ERR_PARA_INVALID;

			pCmd->pExt->transferData = (uint8*)&g_Ble.batDesc[port];
			pCmd->pExt->transferLen = sizeof(BatteryDesc);
		}
		else if (pCmd->cmd == REQ_ID_RESET)
		{
			//Mcu reset, To do... 
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

uint8* Ble_ReqProc(const uint8_t* pReq, int frameLen, uint8* rspLen)
{
	return BleTpu_ReqProc(&g_BleTpu, pReq, frameLen, rspLen);
}

int Ble_txData(uint8_t cmd, const uint8_t* pData, int len)
{
	return len;
}

void Ble_init(uint8* mac)
{
#define BLE_CMD_SIZE 13
	static UtpCmdEx g_JtCmdEx[BLE_CMD_SIZE];
	static uint8 rxBuf[32];
	static uint8 txBuf[64];
	static const UtpCmd g_UtpCmds[BLE_CMD_SIZE] =
	{
		{&g_JtCmdEx[0],UTP_EVENT, REQ_ID_AUTHR			 , "Authr"       , Null, 0, rxBuf  , sizeof(rxBuf),(UtpEventFn)Ble_authent},
		{&g_JtCmdEx[0],UTP_EVENT, REQ_ID_GET_DEVICEID    , "GetDevID"    , Null, 0, (uint8*)& g_Ble.devIdPkt  , sizeof(BleGetDevIDPkt)},
		{&g_JtCmdEx[1],UTP_EVENT, REQ_ID_GET_PORT_STATE  , "GetPortState",Null, 0, (uint8*)& g_Ble.portState , sizeof(PmsPortStatePkt)},
		{&g_JtCmdEx[2],UTP_EVENT, REQ_ID_GET_BATTERYINFO , "GetBatInfo"  , rxBuf, 1},
		{&g_JtCmdEx[3],UTP_EVENT, REQ_ID_GET_TESTRESULT  , "GetSelfTest" , Null, 0, txBuf, sizeof(SelfTestResult), (UtpEventFn)Ble_getSelfTestResult},
		{&g_JtCmdEx[4],UTP_EVENT, REQ_ID_GET_GPS_GPRS_INFO,"GetGpsInfo"  , Null, 0, (uint8*)& g_Ble.gpsPkt, sizeof(GpsPkt)},
		{&g_JtCmdEx[5],UTP_EVENT, REQ_ID_GET_PMS_INFO	  ,"GetPmsInfo"  , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		{&g_JtCmdEx[6],UTP_EVENT, REQ_ID_RESET	  ,"DevReset"},
		{&g_JtCmdEx[7],UTP_EVENT, REQ_ID_SET_NVDS ,"SetNvds", (uint8*)& rxBuf, sizeof(rxBuf), Null, 0, (UtpEventFn)Ble_setNvds},
		//{&g_JtCmdEx[8],UTP_EVENT, REQ_ID_SET_FACTORY_SETTINGS ,"RecoverFactoryCfg", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[9],UTP_EVENT, REQ_ID_GET_NVDS		  ,"GetNvds"     , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[10],UTP_EVENT, REQ_ID_ACTIVE_REQ	  ,"DevActive"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[11],UTP_EVENT, REQ_ID_BAT_VERIFY	  ,"BatVerify"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[12],UTP_EVENT, REQ_ID_SET_ALARM_MODE,"SetAlarmMode", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
	};
	static const UtpCfg cfg =
	{
		.cmdCount = BLE_CMD_SIZE,
		.cmdArray = g_UtpCmds,
		.TxFn = Ble_txData,
		.TresferEvent = (UtpEventFn)Ble_utpEventCb,
		.pCbObj = &g_Ble,
	};

	static uint8_t g_JtUtp_txBuff[128];			//发送数据缓冲区
	static uint8_t g_JtUtp_rxBuff[200];			//接收数据缓冲区
	const BleTpuFrameCfg g_BleFrameCfg =
	{
		//帧特征配置
		.cmdByteInd = 0,
		.dataByteInd = 2,

		//分配协议缓冲区
		.txBufLen = sizeof(g_JtUtp_txBuff),
		.txBuf = g_JtUtp_txBuff,
		.rxBufLen = sizeof(g_JtUtp_rxBuff),
		.rxBuf = g_JtUtp_rxBuff,

		//返回码定义
		.result_SUCCESS = SUCCESS,
		.result_UNSUPPORTED = ERR_UNSUPPORTED,
	};

	//初始化g_Ble.portState变量
	g_Ble.portState.portCount = 2;
	g_Ble.portState.property[0].portNum = 0;
	g_Ble.portState.property[1].portNum = 1;

	BleTpu_Init(&g_BleTpu, &cfg, &g_BleFrameCfg);
	BleUser_init(&g_Ble.user, mac);
}

