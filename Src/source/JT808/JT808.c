
#include "Common.h"
#include "JT808.h"
#include "MsgDef.h"
#include "cirbuffer.h"
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
//static uint32_t g_hbIntervalMs = 2000;	//MCU心跳时间间隔，单位Ms
static uint8_t g_txBuf[128*2];	//100
static uint16_t g_txlen = 0;
uint8_t g_hbdata[4] = {0x00, 0x00, 0x07, 0xD0};

// Jose add
static JtDevBleCfgParam g_BleCfgParam_mirror ;
static uint8_t gSendCanCmdCnt = 0 ;	// 发送计数---连接失败多次,则重启之...
static uint32_t gCanbusRevTimeMS = 0;
#define			_RESEND_CMD_COUNT		(2)	// 重发3

// GPS时间---用以记录 GPS定位用时时间
static uint32 	gGPSTimeCnt ;		// 
static uint32 	gGPSFixCnt;			// GPS 定时时间
// GPRS时间---用以记录GPRS从发送到接收时间
static uint32 	gGPRSSendTimeCnt;
static uint32 	gGPRSConTimeCnt ;		// 连接用时时间
static uint32	gGPRSConCnt;			// GPRS 连接时间

uint16_t gCurRevLen = 0 ;

extern void can0_reset(void);


#define		_MODULE_SET_GPRS_STATE		do{g_Jt.devState.cnt |= _NETWORK_CONNECTION_BIT ;}while(0)	
#define		_MODULE_CLR_GPRS_STATE		do{g_Jt.devState.cnt &= ~_NETWORK_CONNECTION_BIT ;}while(0)
#define		_MODULE_SET_GPS_STATE		do{g_Jt.devState.cnt |= _GPS_FIXE_BIT ;}while(0)
#define		_MODULE_CLR_GPS_STATE		do{g_Jt.devState.cnt &= ~_GPS_FIXE_BIT ;}while(0)



void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2);

static JT808fsmFn JT808_findFsm(JT_state state);

static void _SetOperationState(uint8_t Operation, uint8_t Parameter );

void Sim_Dump(void)
{
	void JtTlv8103IP_Dump();
	
	#define PRINTF_SIMID(_field) Printf("\t%s=%d\n", #_field, g_Jt.property._field);
	#define PRINTF_GPRSLOC(_field) Printf("\t%s=%d\n", #_field, g_Jt.locatData._field);
	
	Printf("Sim dump:\n");
	
	PRINTF_SIMID(protocolVer);
	PRINTF_SIMID(devClass);
	
	Printf("\tICCID = %s\n", g_Jt.property.iccid);
	Printf("\tSimhwVer = %s\n", g_Jt.property.hwVer);
	Printf("\tSimfwVer = %s\n", g_Jt.property.fwVer);
	Printf("\tID = %s\n", g_Jt.property.devId);
		
	JtTlv8103IP_Dump();
}

void Gprs_Dump(void)
{
	#define PRINTF_GPRSDEVSTA(_field) Printf("\t%s=%d\n", #_field, g_Jt.devState._field);
	#define PRINTF_GPRSLOC(_field) Printf("\t%s=%d\n", #_field, g_Jt.locatData._field);
	
	Printf("Gprs dump:\n");
	
	Printf("\topState=%d\n",g_Jt.opState);
	
	PRINTF_GPRSDEVSTA(cnt);
	PRINTF_GPRSDEVSTA(csq);
	PRINTF_GPRSDEVSTA(snr);
	PRINTF_GPRSDEVSTA(siv);
	
	PRINTF_GPRSLOC(longitude);
	PRINTF_GPRSLOC(latitude);
	
}

void Ble_Dump(void)
{
	#define PRINTF_BLEDEVSTA(_field) Printf("\t%s=%d\n", #_field, g_Jt.bleproperty._field);
	#define PRINTF_BLEDEVCFG(_field) Printf("\t%s=%d\n", #_field, g_Jt.blecfgParam._field);
	Printf("Ble dump:\n");

	Printf("\tBlehwVer = %s\n", g_Jt.bleproperty.BlehwVer);
	Printf("\tBlefwVer = %s\n", g_Jt.bleproperty.BlefwVer);	
	PRINTF_BLEDEVSTA(BleType);
	Printf("\tMAC = [%s]\n", g_Jt.bleproperty.BleMac);

	//Printf("\tBleName = %s\n", g_Jt.blecfgParam.BleName);
	
	PRINTF_BLEDEVCFG(BleAdvInterval);
	PRINTF_BLEDEVCFG(BleAdvPower);
	
	Printf("\tbleState=%d\n",g_Jt.bleState.bleConnectState);
	Printf("\tbleConnectMAC=%s\n",g_Jt.bleState.bleConnectMAC);
	
	

}


UTP_EVENT_RC JT808_cmd_getSimID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevProperty) - JT_DEV_HW_VER_SIZE - JT_DEV_FW_VER_SIZE;

	if (ev == UTP_REQ_SUCCESS)
	{
		uint8 ver[JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE];

		//接收到数据和property定义的结构不匹配，需要重新赋值HwVer和FwVer

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

		// 大小端转换
		g_Jt.property.protocolVer = Dt_convertToU16( &g_Jt.property.protocolVer , DT_UINT16 );
		g_Jt.property.devClass = Dt_convertToU32( &g_Jt.property.devClass , DT_UINT32 );

		// Info
		//PFL(DL_JT808,"SIM ID\r\n");
		//PFL(DL_JT808,"ver:%02X\r\n",g_Jt.property.protocolVer);
		//PFL(DL_JT808,"Calss:%d\r\n",g_Jt.property.devClass);
		//PFL(DL_JT808,"devModel:%s\r\n",g_Jt.property.devModel);
		//PFL(DL_JT808,"devID:%s\r\n",g_Jt.property.devId);
		//PFL(DL_JT808,"ICCID:%s\r\n",g_Jt.property.iccid);
		//PFL(DL_JT808,"hwVer:%s\r\n",g_Jt.property.hwVer);
		//PFL(DL_JT808,"fwVer:%s\r\n",g_Jt.property.fwVer);

		gSendCanCmdCnt = 0 ;	// 清0
	}
	else if (ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Read sim param failed.\r\n" );
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			PFL_WARNING("Resend Cmd Timeout,can reset\r\n");
			can0_reset();
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_SIM_ID,gSendCanCmdCnt);
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);
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
		//const TlvIn* p = &g_jtTlvInMgr_8103.itemArray[readParamOffset];
		const TlvOut *p = &g_jtTlvOutMgr_8103.itemArray[readParamOffset];
		//配置读取参数
		for (i = 0; (i + readParamOffset) < g_jtTlvOutMgr_8103.itemCount && i < 5; i++, p++)
		{
			paramIDs[i] = p->tag;
		}
		pCmd->pExt->transferLen = i * 2;
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		//UTP_FSM_WAIT_RSP == pUtp->state 
		readParamOffset += i;
		JtTlv8103_updateMirror( &pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1 );
		//JtTlv8103_updateStorage(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		if (readParamOffset < g_jtTlvOutMgr_8103.itemCount )//  g_jtTlvInMgr_8103.itemCount)
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
		}
		else //全部读取完毕
		{
			//i = 0;
			readParamOffset = 0;
			// 要发送数据
			// 如果 URL or port 变化，则修改之
			//
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG );
		}
		i = 0 ;
		gSendCanCmdCnt = 0x00 ;
	}
	else if (ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		readParamOffset = 0 ;
		i = 0 ;
		PFL_WARNING("Write sim param failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0x00;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,can reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_SIM_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);	// 一定要获取到
	}

	return UTP_EVENT_RC_SUCCESS;
}


// Joe add 2020/9/17


UTP_EVENT_RC JT808_cmd_setSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_TX_START )
	{
		uint8_t tlvCount = 0 ;
		pCmd->pExt->transferLen = JtTlv8103_getChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen , &tlvCount );
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED  )	//读取失败，重新读取
	{
		PFL_WARNING("Write sim CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0x00;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_SIM_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG);
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_setBleCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	/*
	//if (ev == UTP_TX_START)
	//{
	//	pCmd->pExt->transferLen = sizeof( JtDevBleCfgParam );
	//	memcpy( (uint8_t*)pCmd->pExt->transferData ,&g_Jt.blecfgParam , sizeof( JtDevBleCfgParam ) );
	//}
	else */if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Write BLE CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_SIM_CFG,gSendCanCmdCnt);			
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getBleEnable(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_REQ_SUCCESS )
	{
		//_JT808_DEBUGMSG("BLE Enable:%d\r\n",pJt->bleEnCtrl);
		if( !(pJt->bleEnCtrl & 0x01) )
		{
			//_JT808_DEBUGMSG("Enable BLE\r\n");
			pJt->bleEnCtrl = 0x01 ;
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
		}
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Get BLE Enable failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,can reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_BLE_EN,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_setBleEnable(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	/*if (ev == UTP_TX_START )
	//{
	//	pJt->bleEnCtrl = 0x01 ;
	//	memcpy( (uint8_t*)pCmd->pExt->transferData, &pJt->bleEnCtrl , 2 );
	//	pCmd->pExt->transferLen = 2 ;
	//}
	else */if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Set BLE Enable failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_BLE_EN,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_cmd_getBleID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevBleProperty) - JT_DEV_BLE_HW_VER_SIZE - JT_DEV_BLE_FW_VER_SIZE;
	if (ev == UTP_REQ_SUCCESS)
	{
		uint8 ver[JT_DEV_BLE_HW_VER_SIZE + JT_DEV_BLE_FW_VER_SIZE];

		//接收到数据和property定义的结构不匹配，需要重新赋值HwVer和FwVer

		memcpy(ver, &g_Jt.bleproperty.BlehwVer, pCmd->pExt->transferLen - headSize - 1);
		memset(&g_Jt.bleproperty.BlehwVer, 0, JT_DEV_BLE_HW_VER_SIZE + JT_DEV_BLE_FW_VER_SIZE );

		if (ver[0] < JT_DEV_BLE_HW_VER_SIZE )
		{
			memcpy(&g_Jt.bleproperty.BlehwVer, &ver[1], ver[0]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 BlehwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_BLE_FW_VER_SIZE )
		{
			memcpy(&g_Jt.bleproperty.BlefwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 BlefwVer size(%d) error.\n", ver[offset]);
		}
		g_Jt.bleproperty.BleType = Dt_convertToU16(&g_Jt.bleproperty.BleType,DT_UINT16);
		
		gSendCanCmdCnt = 0x00 ;
		//PFL(DL_JT808,"BLE:%02X[%02X%02X%02X%02X%02X%02X]\r\n",
		//	g_Jt.bleproperty.BleType , 
		//	g_Jt.bleproperty.BleMac[0],g_Jt.bleproperty.BleMac[1],
		//	g_Jt.bleproperty.BleMac[2],g_Jt.bleproperty.BleMac[3],
		//	g_Jt.bleproperty.BleMac[4],g_Jt.bleproperty.BleMac[5]);
		//PFL(DL_JT808,"BLE Ver:%s---%s\r\n",
		//s	g_Jt.bleproperty.BlehwVer,g_Jt.bleproperty.BlefwVer );
		//
		Ble_init( g_Jt.bleproperty.BleMac );
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Get BLE ID failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0x00;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_BLE_ID,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_BLE_ID );
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_cmd_getBleCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	static int readParamOffset = 0;
//	static int i = 0;

	if (ev == UTP_TX_START)		// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		g_BleCfgParam_mirror.BleAdvInterval = Dt_convertToU32(&g_BleCfgParam_mirror.BleAdvInterval,DT_UINT32);
		//g_BleCfgParam_mirror
		//_JT808_DEBUGMSG("BLE\r\nName:%s",
		//	g_BleCfgParam_mirror.BleName );
		//_JT808_DEBUGMSG("Inter:%d,Power:%d\r\n",
		//	g_BleCfgParam_mirror.BleAdvInterval,g_BleCfgParam_mirror.BleAdvPower );
		//_JT808_DEBUGMSG("Adv:%s\r\n",g_BleCfgParam_mirror.BleAdvData);
		// 查看是否需要设置BLE Name
		//JtDevBleCfgParam
		if( 0 != memcmp( &g_BleCfgParam_mirror , &g_Jt.blecfgParam ,sizeof(JtDevBleCfgParam) ))
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
		}
		gSendCanCmdCnt = 0x00;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Get BLE CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_BLE_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_BLE_CFG );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getFileInfo(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;
	//
	if (ev == UTP_TX_START)		// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		g_Jt.updatefileinfo.Updatefilelength = Dt_convertToU32(&g_Jt.updatefileinfo.Updatefilelength,DT_UINT32);

		//_JT808_DEBUGMSG("Update file Len:%d,Ver:%d\r\n",
		//	g_Jt.updatefileinfo.Updatefilelength,g_Jt.updatefileinfo.UpdatefileVersion );
		gSendCanCmdCnt = 0 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Get UpdateFile Info failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_FILE_INFO,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_FILE_INFO );
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_cmd_setToOpState(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	static int readParamOffset = 0;
//	static int i = 0;
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;
	//
	if (ev == UTP_TX_START) 	// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS) //读取成功
	{
		gSendCanCmdCnt = 0 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		PFL_WARNING("Set To OpState failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,reboot\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_SET_OP_STATE,gSendCanCmdCnt);
			
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_SET_OP_STATE );
	}
	return UTP_EVENT_RC_SUCCESS;
}

static void _SetOperationState(uint8_t Operation, uint8_t Parameter )
{
	g_Jt.setToOpState.OperationState = Operation ;
	g_Jt.setToOpState.StateParameter = Parameter ;
	Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
}




void JT808_switchState(JT808* pJt, JT_state newState)
{
	if (pJt->opState == newState) return;
	
	switch (newState)
	{
		case JT_STATE_INIT:
			// 发送心跳
			Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
			_SetOperationState( JT_STATE_PREOPERATION,
				_GPRS_LOWMODE | _GPS_LOWMODE | _BLE_LOWMODE );

			// 清除所有状态
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			break;
		case JT_STATE_SLEEP:
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			break;
		case JT_STATE_WAKEUP:
			// 发送心跳
			Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
			_SetOperationState(JT_STATE_PREOPERATION,
				_GPRS_LOWMODE | _GPS_LOWMODE | _BLE_LOWMODE );

			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			
			break;
		case JT_STATE_PREOPERATION:
			// 发送心跳
			Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
			
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);// 0x11
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_ID); //0x30
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_CFG); //0x31
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN); // 0x33			
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);//0x16

			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
				
			break;
		case JT_STATE_OPERATION:
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
			break;
	}

	pJt->fsm = JT808_findFsm(newState);
}

UTP_EVENT_RC JT808_event_simHb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_CHANGED_BEFORE)
	{
		JT_state newOp = *pCmd->pExt->transferData;
		//_JT808_DEBUGMSG("SIMHb:%d-%d\r\n",pJt->opState,newOp);
		JT808_switchState(pJt, newOp);
	}
	else if( ev == UTP_CHANGED_AFTER )
	{
		// 通信模组状态与 本地保存状态不一至时
		if( pJt->opState != pJt->setToOpState.OperationState )
		{
			pJt->opState = JT_STATE_INIT;		// 重新开始
			_SetOperationState(JT_STATE_PREOPERATION,
				_GPRS_LOWMODE | _GPS_LOWMODE | _BLE_LOWMODE );
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			//PFL(DL_JT808,"Change OPState[%d-%d]\r\n",pJt->opState ,pJt->setToOpState.OperationState );
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_devStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		g_Jt.devState.cnt = Dt_convertToU16(&g_Jt.devState.cnt,DT_UINT16);
		if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT )
		{
			PFL(DL_JT808,"Network Connected\r\n");
		}
		else
		{
			PFL(DL_JT808,"Network disconnect\r\n");
		}
		if( g_Jt.devState.cnt & _GPS_FIXE_BIT )
		{
			PFL(DL_JT808,"GPS Fixe\r\n");
		}
		else
		{
			PFL(DL_JT808,"GPS No Fixe\r\n");
		}
		
		if( g_Jt.devState.cnt & _SMS_EXIT_BIT )
		{
			PFL(DL_JT808,"Rev SMS\r\n");
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );// 获取短信
		}
		else
		{
			PFL(DL_JT808,"No SMS\r\n");
		}
		//
		PFL(DL_JT808,"CSQ:%d,SNR:%d,SView:%d\r\n",g_Jt.devState.csq,g_Jt.devState.snr,g_Jt.devState.siv);
		
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_getSMSContext(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		// 处理短信

		// 还有短信
		if( g_Jt.smsContext.smsExist )
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_LocationChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		g_Jt.locatData.latitude = Dt_convertToU32(&g_Jt.locatData.latitude,DT_UINT32);
		g_Jt.locatData.longitude = Dt_convertToU32(&g_Jt.locatData.longitude,DT_UINT32);
		
		PFL(DL_JT808,"lat:%d,long:%d\r\n",
			g_Jt.locatData.latitude,g_Jt.locatData.longitude)
	}
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC JT808_event_rcvSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	 if (ev == UTP_CHANGED_AFTER)
	 {
	  	JtTlv8900_proc(pCmd->pExt->transferData, pCmd->pExt->transferLen);
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_event_rcvFileData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	 if (ev == UTP_CHANGED_AFTER)
	 {
	 	// 处理数据
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	//
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

//

UTP_EVENT_RC JT808_event_sendSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	static int readParamOffset = 0;
//	static int i = 0;
//	int len = 0;
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;

	if(ev == UTP_TX_START)	
	{
		pCmd->pExt->transferLen = g_txlen;
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		g_txlen = 0;
		int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
		JtTlv0900_updateMirror( g_txBuf , len );
		PFL(DL_JT808,"0900 UpdataMirror\r\n");
	}

	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getFileContent(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if(ev == UTP_CHANGED_AFTER )
	{
		//Offset[UINT32] + file Data[UINT8[]
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_event_setLocationExtras(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC JT808_event_rcvBleData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{	
	// 应该处理 UTP_CHANGED_AFTER 事件
	if( ev == UTP_CHANGED_AFTER )	// 收到数据
	{
		//pCmd->pExt->transferData = (uint8*)pData;
		//pCmd->pExt->transferLen = frameLen;
	}
	else if (ev == UTP_GET_RSP)		// 返回数据
	{
		// 将返回数据写入 pCmd->pExt->transferData		  & pCmd->pExt->transferLen及可
		uint8 rspLen = 0; //extern uint16_t gCurRevLen ;
		pCmd->pExt->transferData = Ble_ReqProc(pCmd->pStorage, gCurRevLen, &rspLen);
		pCmd->pExt->transferLen = rspLen;
		PFL(DL_JT808,"BLE Send:%d\r\n", rspLen);
	}
	return UTP_EVENT_RC_SUCCESS;
}

//
UTP_EVENT_RC JT808_event_bleStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		if( g_Jt.bleState.bleConnectState & _BLE_CONNECT_BIT )
		{
			PFL(DL_JT808,"BLE Connect\r\n");
		}
		else
		{
			PFL(DL_JT808,"Ble Disconnect\r\n");
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_bleAuthChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
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
		if (ev == UTP_REQ_SUCCESS)
		{
			Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);
		}
	}
	else if (pCmd->cmd == JTCMD_CMD_SEND_TO_SVR)
	{
		//if (ev == UTP_REQ_SUCCESS)
		//{
		//	JtTlv0900_updateMirror(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		//}
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
			_SetOperationState(JT_STATE_OPERATION,
				_GPRS_NORMAL | _GPS_NORMAL | _BLE_NORMAL );
		}
	}
}

void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_RUN)
	{
		// 更新JtTlv9000数据
		JtTlv0900_updateStorage();	//
		
		if (Utp_isIdle(&g_JtUtp) && (g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT) )	// 空闲 & 连接网络s
		{
			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
			if (len) 
			{
				g_txlen = len;		// 全局数据
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
				PFL(DL_JT808,"Send Bat Info:%d\r\n",len );
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

	//程序不可能运行到这里
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
		case 15: printf("receive Utpcmd15!\r\n");rt_thread_mdelay(1); g_txlen = sprintf(g_txBuf,"Hello Server"); ;Utp_SendCmd(&g_JtUtp, 0x15);break;
	}
 
}


MSH_CMD_EXPORT(JTcmd, JT808_sendCmd<uint8_t ind>);




#define	_CAN_TX_TIMEOUT_MS		(30)
extern can_trasnmit_message_struct transmit_message;
extern can_receive_message_struct  receive_message;
extern FlagStatus can0_receive_flag;
//xx 发送数据到总线 一次最多发送8个字节 发送时需判断是否到最后一包
int JT808_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmd为CAN协议的PF
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
		transmit_message.tx_efid = 0x00002010;
		can_message_transmit(CAN1, &transmit_message);
		PFL(DL_CAN, "CANTx:");
		PFL(DL_CAN, "%08x ",transmit_message.tx_efid);
		DUMP_BYTE_LEVEL(DL_CAN,&transmit_message.tx_data,transmit_message.tx_dlen);
		PFL(DL_CAN, "\n");
//		rt_thread_mdelay(20);
		tx_timeout = GET_TICKS();
		while(
			CAN_TRANSMIT_PENDING == can_transmit_states(CAN1, CAN_MAILBOX0) &&
			GET_TICKS() - tx_timeout <_CAN_TX_TIMEOUT_MS );

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

void JT808_rxDataProc(const uint8_t* pData, int len)
{
	//Utp_RxData(&g_JtUtp, pData, len);
	CirBuffPush( &g_JtUtp.rxBuffCirBuff, pData ,len );
}

// GPS 1s一条,心跳2s一条
#define			_CAN_BUS_REV_TIMEOUT_MS			(1000*6)

void JT808_timerProc()
{
	if( can0_receive_flag == SET )
	{
		can0_receive_flag = RESET;
		PFL(DL_CAN, "CANRx:");
		PFL(DL_CAN, "%08x ",receive_message.rx_efid);
		DUMP_BYTE_LEVEL(DL_CAN,&receive_message.rx_data,receive_message.rx_dlen);
		PFL(DL_CAN, "\n");
		if(0x00001020 == receive_message.rx_efid)
		{
			JT808_rxDataProc( receive_message.rx_data , receive_message.rx_dlen );
			gCanbusRevTimeMS = GET_TICKS();
		}
		else if(0x00001030 == receive_message.rx_efid)
		{
		
		}
		//JT808_rxDataProc( receive_message.rx_data , receive_message.rx_dlen );
		//gCanbusRevTimeMS = GET_TICKS();
		//PFL(DL_JT808,"CAN Rev len:%d\n",receive_message.rx_dlen );
	}
	else
	{
		if( GET_TICKS() - gCanbusRevTimeMS > _CAN_BUS_REV_TIMEOUT_MS )
		{
			PFL_WARNING("CAN Rev Timeout.reset can\r\n");
			can0_reset();
			gCanbusRevTimeMS = GET_TICKS();
			// 清除所有状态
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
		}
	}
}

void JT808_run()
{
	JT808_timerProc();
	JT808_fsm(MSG_RUN, 0, 0);
	#ifdef DGT_CONFIG	
	WDOG_Feed();
	#endif
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

	gCanbusRevTimeMS = GET_TICKS();			
}

static void _UpdataBleAdvData(void)
{
	uint8_t _adv[31+31] ={0x00};
	uint8_t adv_type[] = {0x02,0x01,0x05};
	uint8_t adv_uuid[] = {0x11,0x07,0x39,0x23,0xCF,0x40,0x73,0x16,0x42,0x9A,0x5C,0x41,0x7E,0x7D,0xC4,0x9A,0x83,0x14};
	uint8_t adv_comp1[] = {0x07,0xFF,0x2E,0xCE,0x8C,0x42,0x4C,0x57};
	//uint8_t adv_comp2[] = {0x03,0xFF,0x33,0x30};
	uint8_t i = 0 ;
	memset( _adv , 0 , sizeof(_adv) );

	// 类型
	memcpy( _adv + i , adv_type , sizeof(adv_type) );
	i += sizeof(adv_type);
	//UUID
	memcpy( _adv + i , adv_uuid , sizeof(adv_uuid) );
	i += sizeof(adv_uuid);
	//公司
	memcpy( _adv + i , adv_comp1 , sizeof(adv_comp1) );
	i += sizeof(adv_comp1);
	// Name
	_adv[i++] = 1 + sizeof(_BLE_NAME) - 1;
	_adv[i++] = 0x09;	// name
	memcpy( _adv + i , _BLE_NAME , sizeof(_BLE_NAME) - 1 );
	i += ( sizeof(_BLE_NAME) - 1 );
	// 公司
	_adv[i++] = 1 + sizeof(_BLE_COMPY) - 1 ;
	_adv[i++] = 0xFF ;
	memcpy( _adv + i , _BLE_COMPY , sizeof(_BLE_COMPY) - 1);
	i += ( sizeof(_BLE_COMPY) - 1 );

	memcpy( g_Jt.blecfgParam.BleAdvData , _adv , sizeof(_adv));
}

void JT808_init()
{
	
/************************************************
*以下变量是协议变量，仅供协议使用
************************************************/
	
	#define JT_CMD_SIZE 23
	static uint8_t g_protocolVer = 1;	//传输协议版本号
	static uint8_t g_updatefiletype = 1; // 中控
	static uint8_t g_rxBuf[192];	
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//位置越靠前，发送优先级越高
		{&g_JtCmdEx[0],UTP_NOTIFY, JTCMD_MCU_HB, "McuHb", g_hbdata, sizeof(g_hbdata),Null,0,Null},	/*(uint8_t*)& g_hbIntervalMs*/
		{&g_JtCmdEx[1],UTP_EVENT_NOTIFY, JTCMD_SIM_HB, "SimHb", (uint8_t*)& g_Jt.opState, 1, Null, 0, (UtpEventFn)JT808_event_simHb},
		//	
		{&g_JtCmdEx[2],UTP_READ , JTCMD_CMD_GET_SIM_ID , "GetSimID"	, (uint8_t*)& g_Jt.property, sizeof(JtDevProperty), &g_protocolVer, 1, (UtpEventFn)JT808_cmd_getSimID},
		{&g_JtCmdEx[3],UTP_READ , JTCMD_CMD_GET_SIM_CFG, "GetSimCfg", (uint8_t*)& g_rxBuf, sizeof(g_rxBuf), (uint8_t*)& g_txBuf, 0/*sizeof(g_txBuf)*/, (UtpEventFn)JT808_cmd_getSimCfg},
		{&g_JtCmdEx[4],UTP_WRITE, JTCMD_CMD_SET_SIM_CFG, "SetSimCfg", (uint8_t*)& g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_cmd_setSimCfg},
		{&g_JtCmdEx[5],UTP_READ , JTCMD_CMD_GET_BLE_ID , "GetBLEID",(uint8_t*)& g_Jt.bleproperty, sizeof(JtDevBleProperty),Null,0,(UtpEventFn)JT808_cmd_getBleID},
		{&g_JtCmdEx[6],UTP_READ , JTCMD_CMD_GET_BLE_CFG , "GetBLECfg",(uint8_t*)& g_BleCfgParam_mirror, sizeof(JtDevBleCfgParam),Null,0,(UtpEventFn)JT808_cmd_getBleCfg},
		{&g_JtCmdEx[7],UTP_WRITE, JTCMD_CMD_SET_BLE_CFG, "SetBLECfg", (uint8_t*)&g_Jt.blecfgParam , sizeof( JtDevBleCfgParam ), Null, 0, (UtpEventFn)JT808_cmd_setBleCfg},
		{&g_JtCmdEx[8],UTP_READ, JTCMD_CMD_GET_BLE_EN, "GetBLEEnable", (uint8_t*)& g_Jt.bleEnCtrl, 2,Null, 0, (UtpEventFn)JT808_cmd_getBleEnable},
		{&g_JtCmdEx[9],UTP_WRITE, JTCMD_CMD_SET_BLE_EN, "SetBLEEnable", (uint8_t*)&g_Jt.bleEnCtrl , 2, Null, 0, (UtpEventFn)JT808_cmd_setBleEnable},
		{&g_JtCmdEx[10],UTP_READ , JTCMD_CMD_GET_FILE_INFO , "GetFileInfo",(uint8_t*)&g_Jt.updatefileinfo , sizeof(UpdateFileInfo),(uint8_t*)&g_updatefiletype,sizeof(g_updatefiletype),(UtpEventFn)JT808_cmd_getFileInfo },
		{&g_JtCmdEx[11],UTP_WRITE , JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.setToOpState, 1 , Null , 0 , (UtpEventFn)JT808_cmd_setToOpState},
		{&g_JtCmdEx[12],UTP_WRITE, JTCMD_CMD_SEND_TO_SVR, "SendDataToSvr", (uint8_t*)g_txBuf, sizeof(g_txBuf), Null, 0, (UtpEventFn)JT808_event_sendSvrData},
		{&g_JtCmdEx[13],UTP_READ,JTCMD_CMD_GET_SMS,"GetSMSContext",(uint8_t*)&g_Jt.smsContext,sizeof(GetSMSContext),Null,0,(UtpEventFn)JT808_event_getSMSContext},
		{&g_JtCmdEx[14],UTP_READ , JTCMD_CMD_GET_FILE_CONTENT, "GetFileContent"	, (uint8_t*)g_rxBuf, sizeof(g_rxBuf), (uint8_t*)& g_Jt.filecontent, sizeof(FileContent),(UtpEventFn)JT808_cmd_getFileContent},
		{&g_JtCmdEx[15],UTP_WRITE, JTCMD_CMD_SET_LOCATION_EXTRAS, "SetLocationExtras", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_setLocationExtras},

		// EVENT 
		// 网络 & GPS
		{&g_JtCmdEx[16],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},
		{&g_JtCmdEx[17],UTP_EVENT, JTCMD_EVENT_DEV_STATE_LOCATION, "LocationChanged", (uint8_t*)& g_Jt.locatData, sizeof(Jt_LocationData), Null, 0, (UtpEventFn)JT808_event_LocationChanged},
		{&g_JtCmdEx[18],UTP_EVENT, JTCMD_EVT_RCV_SVR_DATA, "RcvSvrData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvSvrData},
		{&g_JtCmdEx[19],UTP_EVENT, JTCMD_EVT_RCV_FILE_DATA, "RcvFileData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvFileData},
		// 蓝牙
		{&g_JtCmdEx[20],UTP_EVENT, JTCMD_BLE_EVT_AUTH, "BleAuth", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null , 0 , (UtpEventFn)JT808_event_bleAuthChanged },
		{&g_JtCmdEx[21],UTP_EVENT, JTCMD_BLE_EVT_CNT, "BleCnt", (uint8_t*)&g_Jt.bleState, sizeof(Jt_BleState) , Null , 0 , (UtpEventFn)JT808_event_bleStateChanged },
		// 
		{&g_JtCmdEx[22],UTP_EVENT, JTCMD_BLE_RCV_DAT, "BleRcvDat", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), (uint8_t*)g_txBuf, 0/*sizeof(g_txBuf)*/, (UtpEventFn)JT808_event_rcvBleData},
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

	{
		_UpdataBleAdvData();
		g_Jt.blecfgParam.BleAdvInterval = 30;	
		g_Jt.bleEnCtrl = 0x01 ;
	}

	Utp_Init(&g_JtUtp, &g_cfg, &g_jtFrameCfg);

	JtTlv8900_init();
	JtTlv0900_init();
	JtTlv8103_init();
	
}


