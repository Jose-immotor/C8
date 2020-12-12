
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
#include "JT808.h"
#include "debug.h"

#ifdef CANBUS_MODE_JT808_ENABLE	


//spCmd->pExt->transferData && pCmd->pExt->transferLen 


Ble g_Ble;
static BleTpu g_BleTpu;
//extern uint8_t Rs485TestSendFlag ;
extern Battery g_Bat[MAX_BAT_COUNT];
UTP_EVENT_RC Ble_getSelfTestResult(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	//PFL(DL_JT808,"BLE_GetSelfTest:%d\n",ev );
	if (ev == UTP_GET_RSP)
	{
		SelfTestResult*  pResult = (SelfTestResult*)pCmd->pExt->transferData;
		//pResult的结构赋值
		pResult->simState = 1;	//SIM卡状态 -- 默认一直在
		pResult->gprsState = (g_pJt->devState.cnt & _NETWORK_CONNECTION_BIT)?1:0;
		pResult->gpsState = (g_Jt.devState.cnt & _GPS_FIXE_BIT)?1:0;
		pResult->devState = g_pdoInfo.isRemoteAccOn ;
		pResult->_18650Vol = 0;
		pResult->devState2 = g_cfgInfo.isActive;
		pResult->batVerify = 0;
		// 485测试
		pResult->periheral = g_pdoInfo.isRs485Ok == 0x00 ? 0x01 : 0x02 ;	// 485
		
		pCmd->pExt->transferLen = sizeof(SelfTestResult);	// 其实不需要,显示设置一下
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC Ble_getBatteryInfo(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if ( ev == UTP_GET_RSP )
	{
		pCmd->pExt->transferLen = 0 ;
		//输入数据
		switch( pCmd->pStorage[0] )
		{
			case 0:
			case 1:
				memcpy( (BatteryDesc*)pCmd->pExt->transferData , &( g_Ble.batDesc[pCmd->pStorage[0]]) ,sizeof(BatteryDesc) );
				pCmd->pExt->transferLen = sizeof(BatteryDesc);
				break ;
				
			default :
				return (UTP_EVENT_RC)0x0C;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC Ble_authent(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		if (BLE_USER_INVALID == BleUser_login(&pBle->user, (char*)pCmd->pStorage))
		{
			return (UTP_EVENT_RC)ERR_USERID_INVALID;
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}
extern uint8 		gCurServerIndex;
extern void updateServerAddr( uint8_t serindex );
extern void JT808_CheckResetSerAddr(void);

static TlvInEventRc _SetServerAddr(void* pObj, const struct _TlvIn* pItem, TlvInEvent ev)
{
	if( ev == TE_CHANGED_AFTER )
	{
		updateServerAddr( gCurServerIndex );
		JT808_CheckResetSerAddr();
	}
	return TERC_SUCCESS ;
}

UTP_EVENT_RC Ble_setNvds(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		const static TlvIn tlvs[] =
		{
			{"SmtFwVer", 0x01, 7, Null, DT_STRUCT},
			{"PmsFwVer", 0x02, 7, Null, DT_STRUCT},
			{"SmtHwVer", 0x04, 2, Null, DT_STRUCT},
			{"SerAddr", 0x10, 1, (uint8_t*)&gCurServerIndex, DT_UINT8 ,(TlvInEventFn)_SetServerAddr }
		};

		for (int i = 0; i < GET_ELEMENT_COUNT(tlvs); i++)
		{
			if (tlvs[i].tag == pCmd->pStorage[0])
			{
				if (tlvs[i].storage)
				{
					if( tlvs[i].Event ) tlvs[i].Event( pBle,&tlvs[i],TE_CHANGED_BEFORE);
					memcpy(tlvs[i].storage, &pCmd->pStorage[2], MIN(pCmd->pStorage[1], tlvs[i].len));
					if( tlvs[i].Event ) tlvs[i].Event( pBle,&tlvs[i],TE_CHANGED_AFTER);
				}
			}
		}
		pCmd->pExt->transferLen = 0;
	}
	return UTP_EVENT_RC_SUCCESS;
}
//7E D3 01 00 01 01 93 03 38 03 00 7E 
UTP_EVENT_RC Ble_setCabinlock(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x00:	// 座舱锁开锁
				g_pdoInfo.isCanbinLock = 1 ;
				Cabin_UnLock();
				NvdsUser_Write(NVDS_PDO_INFO);		// 保存之
				PFL(DL_JT808,"BLE_CanbinLock:%d\n",g_pdoInfo.isCanbinLock);
				break ;
			case 0x01:	// 座舱锁关锁
				break ;
			default :
				return (UTP_EVENT_RC)ERR_PARA_INVALID ;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}
// 激活
UTP_EVENT_RC Ble_setActiveDev(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)		// 返回
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x00:	//
			case 0x01:
				g_cfgInfo.isActive = pCmd->pStorage[0] ;
				NvdsUser_Write(NVDS_CFG_INFO);
				PFL(DL_JT808,"BLE_Active:%d\n",g_cfgInfo.isActive);
				break ;
			default :
				return (UTP_EVENT_RC)ERR_PARA_INVALID ;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


// 电池身份验证
UTP_EVENT_RC Ble_BatteryVerify(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x00:	//
				g_pdoInfo.IsBatVerifyEn = pCmd->pStorage[0] ;
				NvdsUser_Write(NVDS_PDO_INFO);
				PFL(DL_JT808,"BLE_BatVerify:%d\n",g_pdoInfo.IsBatVerifyEn);
				break ;
			default :
				return (UTP_EVENT_RC)ERR_PARA_INVALID ;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//
extern DrvIo* g_pLockEnIO ;
UTP_EVENT_RC Ble_setWheelLock(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x00:	// 座舱锁开锁
			case 0x01:	// 座舱锁关锁
				g_pdoInfo.isWheelLock = pCmd->pStorage[0];
				if( g_pdoInfo.isWheelLock == 0 )
				{
					PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, False);
				}
				else if( g_pdoInfo.isWheelLock == 1 )
				{
					PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, True);
				}
				NvdsUser_Write(NVDS_PDO_INFO);
				PFL(DL_JT808,"BLE_WheelLock:%d\n",g_pdoInfo.isWheelLock);
				break ;
			default :
				return (UTP_EVENT_RC)ERR_PARA_INVALID ;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//

UTP_EVENT_RC Ble_setACC(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x00:
			case 0x01:
				g_pdoInfo.isRemoteAccOn = pCmd->pStorage[0];//*pItem->storage ;
				if(g_pdoInfo.isRemoteAccOn)
				{
					Pms_postMsg(PmsMsg_accOn, 0, 0);
					LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOn,0);
				}
				else
				{
					Pms_postMsg(PmsMsg_accOff, 0, 0);
					LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOn,0);
				}
				NvdsUser_Write(NVDS_PDO_INFO);
				PFL(DL_JT808,"BLE_ACC:%d\n",g_pdoInfo.isRemoteAccOn);
			break ;
			default :
				return (UTP_EVENT_RC)ERR_PARA_INVALID ;
				break ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC Ble_getNvds(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	//_BLE_DEBUGMSG("getNvds:%x,ev:%d:%d\r\n" , pCmd->pStorage[0] , ev , gCurServerIndex );
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		switch ( pCmd->pStorage[0] )
		{
			case 0x10 :
				otvItem*  pResult = (otvItem*)pCmd->pExt->transferData;
				pResult->item = 0x10;
				pResult->len = 0x01 ;
				pResult->param = gCurServerIndex;
				pCmd->pExt->transferLen = 3;
				break ;
			default :
				break ;
		}
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
		{BLE_USER_ADMIN, REQ_ID_SET_WHEELLOCK},
		{BLE_USER_ADMIN, REQ_ID_SET_CABINLOCK},
		{BLE_USER_ADMIN, REQ_ID_SET_ACC},
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

UTP_EVENT_RC Ble_getGpsGprsInfo(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		GpsPkt*  pResult = (GpsPkt*)pCmd->pExt->transferData;
		//pResult的结构赋值
		pResult->CSQ = g_Jt.devState.csq;
		pResult->Satellites = g_Jt.devState.siv;
		pResult->SNR = g_Jt.devState.snr;
		if( g_Jt.devState.cnt & _GPS_FIXE_BIT )
		{
			pResult->longitude = g_Jt.locatData.longitude;
			pResult->latitude = g_Jt.locatData.latitude;
		}
		else
		{
			pResult->longitude = 0 ;
			pResult->latitude = 0 ;
		}
		pResult->speed = 0;
		pCmd->pExt->transferLen = sizeof( GpsPkt);	// 已经符值了
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC Ble_utpEventCb(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{

	if (ev == UTP_GET_RSP)
	{
		//判断命令是否允许
		if (!Ble_cmdIsAllow(pBle, pCmd->cmd)) return (UTP_EVENT_RC)ERR_CMD_NOT_ALLOW;

		switch( pCmd->cmd )
		{
			case REQ_ID_GET_GPS_GPRS_INFO :
				
				break ;
			case REQ_ID_RESET :
				//SoftReset();		// 重启
				Boot();
				break ;
		}
		
		//if (pCmd->cmd == REQ_ID_GET_PORT_STATE)
		//{
		//	uint8 port = *pCmd->pStorage;
		//	if (port >= MAX_BAT_COUNT) return ERR_PARA_INVALID;

		//	pCmd->pExt->transferData = (uint8*)&g_Ble.batDesc[port];
		//	pCmd->pExt->transferLen = sizeof(BatteryDesc);
		//}
		//else if (pCmd->cmd == REQ_ID_RESET)
		//{
			//Mcu reset, To do... 
		//}
	}

	return UTP_EVENT_RC_SUCCESS;
}

uint8* Ble_ReqProc(const uint8_t* pReq, int frameLen, uint8* rspLen)
{
	if( g_BleTpu.cfg )
	{
		PFL(DL_JT808, "BLE Rx[%d]:",frameLen);
		DUMP_BYTE_LEVEL(DL_JT808, pReq , frameLen );
		PFL(DL_JT808, "\n");
		return BleTpu_ReqProc(&g_BleTpu, pReq, frameLen, rspLen);
	}
	return Null ;
}

int Ble_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)
{
	return len;
}
/*
0x04 --- 获取电池信息   -- REQ_ID_GET_BATTERYINFO ok
0x19 --- 获取版本信息  REQ_ID_GET_DEVICEID     -- ok
0x08 --- 获取 GPS/GPRS 信息 REQ_ID_GET_GPS_GPRS_INFO -- ok
0x07 --- 自检结果				REQ_ID_GET_TESTRESULT -- 
0x2A --- 读取设备Nvds参数  -- REQ_ID_GET_NVDS
0x29 --- 设置设备Nvds参数  -- REQ_ID_SET_NVDS
0x1A --- 读取PMS信息		  -- 不支持
0x7E --- 升级指令			 -- 不支持
0x39 --- 设置ACC			 -- 不支持
0x38 --- 设置座舱锁状态 -- 不支持
*/
void Ble_init(uint8* mac)
{
#define BLE_CMD_SIZE 17
	static UtpCmdEx g_JtCmdEx[BLE_CMD_SIZE];
	static uint8 rxBuf[32];
	static uint8 txBuf[32];
	//
	static const UtpCmd g_UtpCmds[BLE_CMD_SIZE] =
	{
		/*0x01*/{&g_JtCmdEx[0],UTP_EVENT, REQ_ID_AUTHR			 , "Authr"       , rxBuf  , sizeof(rxBuf),txBuf,sizeof(txBuf),(UtpEventFn)Ble_authent},
		/*0x03*/{&g_JtCmdEx[1],UTP_EVENT, REQ_ID_GET_PORT_STATE  , "GetPortState",Null, 0, (uint8*)&g_Ble.portState , sizeof(PmsPortStatePkt)},
		/*0x04*/{&g_JtCmdEx[2],UTP_EVENT, REQ_ID_GET_BATTERYINFO , "GetBatInfo"  , (uint8*)&rxBuf, sizeof(rxBuf),txBuf,sizeof(BatteryDesc),(UtpEventFn)Ble_getBatteryInfo},
		/*0x07*/{&g_JtCmdEx[3],UTP_EVENT, REQ_ID_GET_TESTRESULT  , "GetSelfTest" , Null, 0, txBuf, sizeof(SelfTestResult), (UtpEventFn)Ble_getSelfTestResult},
		/*0x08*/{&g_JtCmdEx[4],UTP_EVENT, REQ_ID_GET_GPS_GPRS_INFO,"GetGpsInfo"  , Null, 0, (uint8*)& g_Ble.gpsPkt, sizeof(GpsPkt),(UtpEventFn)Ble_getGpsGprsInfo},
		/*0x18*/{&g_JtCmdEx[5],UTP_EVENT, REQ_ID_GET_DEVICECAP	 , "GetDevCap"	 , Null, 0, (uint8*)&g_Ble.devCapacity  , sizeof(DevCapacity)},
		/*0x19*/{&g_JtCmdEx[6],UTP_EVENT, REQ_ID_GET_DEVICEID    , "GetDevID"    , Null, 0, (uint8*)&g_Ble.devIdPkt  , sizeof(BleGetDevIDPkt)},
		/*0x29*/{&g_JtCmdEx[7],UTP_WRITE, REQ_ID_SET_NVDS 			,"SetNvds", (uint8*)&rxBuf, sizeof(rxBuf), Null, 0, (UtpEventFn)Ble_setNvds},
		/*0x2A*/{&g_JtCmdEx[8],UTP_EVENT, REQ_ID_GET_NVDS		  ,"GetNvds"     , (uint8*)&rxBuf, sizeof(rxBuf), txBuf,sizeof(txBuf),(UtpEventFn)Ble_getNvds},
		/*0x1A*/{&g_JtCmdEx[9],UTP_EVENT, REQ_ID_GET_PMS_INFO	  ,"GetPmsInfo"  , Null, 0, (uint8*)& g_Ble.devCapacity, sizeof(DevCapacity)},
		/*0x1B*/{&g_JtCmdEx[10],UTP_EVENT, REQ_ID_GET_BMS_INFO	  ,"GetBMSInfo"  , Null, 0, (uint8*)& g_Ble.bmsPkt, sizeof(BleGetDevIDPkt)},
		/*0x1C*/{&g_JtCmdEx[11],UTP_WRITE, REQ_ID_ACTIVE_REQ	  ,"ActiveDev"  , (uint8*)& g_Ble.mActiveDevice, sizeof(uint8) , Null, 0,(UtpEventFn)Ble_setActiveDev },
		/*0x1B*/{&g_JtCmdEx[12],UTP_WRITE, REQ_ID_BAT_VERIFY   ,"BatVerify"	, (uint8*)& g_Ble.BatVerify, 2 * sizeof(uint8) , Null, 0,(UtpEventFn)Ble_BatteryVerify },
		/*0x37*/{&g_JtCmdEx[13],UTP_WRITE, REQ_ID_SET_WHEELLOCK   ,"SetWheellock"	, (uint8*)& g_Ble.wheelLock, sizeof(uint8) , Null, 0,(UtpEventFn)Ble_setWheelLock },
		/*0x38*/{&g_JtCmdEx[14],UTP_WRITE, REQ_ID_SET_CABINLOCK		,"SetCabinloc"	   , (uint8*)&rxBuf, sizeof(rxBuf), txBuf,sizeof(txBuf),(UtpEventFn)Ble_setCabinlock},
		/*0x39*/{&g_JtCmdEx[15],UTP_WRITE, REQ_ID_SET_ACC		,"setACC"	   , (uint8*)&rxBuf, sizeof(rxBuf), txBuf,sizeof(txBuf),(UtpEventFn)Ble_setACC},

		/*0x01*///{&g_JtCmdEx[8],UTP_EVENT, REQ_ID_SET_FACTORY_SETTINGS ,"RecoverFactoryCfg", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		/*0x01*///{&g_JtCmdEx[10],UTP_EVENT, REQ_ID_ACTIVE_REQ	  ,"DevActive"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		/*0x01*///{&g_JtCmdEx[11],UTP_EVENT, REQ_ID_BAT_VERIFY	  ,"BatVerify"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		/*0x01*///{&g_JtCmdEx[12],UTP_EVENT, REQ_ID_SET_ALARM_MODE,"SetAlarmMode", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		/*0x23*/{&g_JtCmdEx[16],UTP_EVENT, REQ_ID_RESET	  			,"DevReset"},
	};
	//
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
	static const BleTpuFrameCfg g_BleFrameCfg =
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
	
	g_Ble.batDesc[0].portId = 0;
	g_Ble.batDesc[1].portId = 1;

	g_Ble.pmsPkt.Version = g_Ble.devIdPkt.protocolVer = 1;
	g_Ble.pmsPkt.HwMainVer = g_Ble.devIdPkt.hwMainVer = DES_HW_VER_MAIN;
	g_Ble.pmsPkt.HwSubVer = g_Ble.devIdPkt.hwSubVer = DES_HW_VER_SUB;
	g_Ble.pmsPkt.AppMainVer = g_Ble.devIdPkt.fwMainVer = FW_VER_MAIN;
	g_Ble.pmsPkt.AppSubVer = g_Ble.devIdPkt.fwSubVer = FW_VER_S1;
	g_Ble.pmsPkt.AppMinorVer = g_Ble.devIdPkt.fwMinorVer = FW_VER_S2;
	g_Ble.pmsPkt.AppBuildNum = g_Ble.devIdPkt.buildNum = FW_VER_BUILD;
	g_Ble.pmsPkt.State = 1;

	//
	g_Ble.devCapacity.protocolVer = g_Ble.devIdPkt.protocolVer = 1;
	/*
	BIT[0]: 是否有 Smart 板； 0-没有， 1-有。
	BIT[1]: 是否有 Pms 板； 0-没有， 1-有。
	BIT[2-3]: 支持最多插入电池数量。
	BIT[4]： 保留。
	BIT[5]：是否支持陀螺仪； 0-不支持， 1-支持。
	BIT[6]：是否支持喇叭； 0-不支持， 1-支持。
	BIT[7]：是否支持轮毂锁； 0-不支持， 1-支持。
	BIT[8]：是否支持座舱锁； 0-不支持， 1-支持。
	BIT[9]：是否支持钥匙点火； 0-不支持， 1-支持。
	BIT[10]：是否有 18650 电池； 0-没有， 1-有。
	BIT[11-31]：保留
	*/
	g_Ble.devCapacity.capacity = BIT(1) | BIT(2) | BIT(7) | BIT(8) | BIT(9) | BIT(10);
	
	
	
	BleTpu_Init(&g_BleTpu, &cfg, &g_BleFrameCfg);
	BleUser_init(&g_Ble.user, mac);
}

#endif // #ifdef CANBUS_MODE_JT808_ENABLE	

