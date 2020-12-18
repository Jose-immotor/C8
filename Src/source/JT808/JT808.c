
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
#include "FirmwareUpdata.h"


/*
{
	// MCU£ºÐ¡¶ËÄ£Ê½
	// CAN Ð­Òé²ÉÓÃ´ó¶ËÄ£Ê½
	// NFC Ð­Òé²ÉÓÃ´ó¶ËÄ£Ê½
	// Rs485²ÉÓÃ´ó¶ËÄ£Ê½´«Êä
	uint32_t no = 0x12345678 ;
	uint8_t *pno = (uint8_t*)&no;
	Printf("no:0x%08X,0x%02X,0x%02X\n",no,no >> 24,pno[0]); //no:0x12345678,0x12,0x78
}
*/


#ifdef CANBUS_MODE_JT808_ENABLE

JT808 g_Jt;
JT808* g_pJt = &g_Jt;
static Utp g_JtUtp;
//static uint32_t g_hbIntervalMs = 2000;	//MCUï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?ï¿½ï¿½Î»MS
static uint8_t g_txBuf[255];	//100
static uint16_t g_txlen = 0;
uint8_t g_hbdata[4] = {0x00, 0x00, 0x07, 0xD0};

// Jose add
static JtDevBleCfgParam g_BleCfgParam_mirror ;
static uint8_t gSendCanCmdCnt = 0 ;	// ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½---ï¿½ï¿½ï¿½ï¿½Ê§ï¿½Ü¶ï¿½ï¿?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö®...
static uint32_t gCanbusRevTimeMS = 0;
#define			_RESEND_CMD_COUNT		(2)	// ï¿½Ø·ï¿½3

// GPSÊ±ï¿½ï¿½---ï¿½ï¿½ï¿½Ô¼ï¿½Â¼ GPSï¿½ï¿½Î»ï¿½ï¿½Ê±Ê±ï¿½ï¿½
static uint32 	gGPSTimeCnt ;		// 
static uint32 	gGPSFixCnt;			// GPS ï¿½ï¿½Ê±Ê±ï¿½ï¿½
// GPRSÊ±ï¿½ï¿½---ï¿½ï¿½ï¿½Ô¼ï¿½Â¼GPRSï¿½Ó·ï¿½ï¿½Íµï¿½ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½
static uint32 	gGPRSSendTimeCnt;
static uint32 	gGPRSConTimeCnt ;		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê±Ê±ï¿½ï¿½
static uint32	gGPRSConCnt;			// GPRS ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½


// sleep 
static uint32  gModuleSleepIngTimeoutCnt ;		// Ä£¿éÇëÇó½øÐÝÃß³¬Ê±¼ÆÊ±
//static uint32  gModuleSleepCnt;					// Ä£¿éÐÝÃß¼ÆÊ±
// wakeup 
static uint32  gModuleWakupIngTimeoutCnt;		// Ä£¿éÇëÇówakeup³¬Ê±¼ÆÊ±
static uint32  gModuleWakeupCnt;				// Ä£¿é»½ÐÑ¼ÆÊ±


uint16_t gCurRevLen = 0 ;

// GPS 1sÒ»ï¿½ï¿½,ï¿½ï¿½ï¿½ï¿½2sÒ»ï¿½ï¿½
#define			_CAN_BUS_REV_TIMEOUT_MS			(1000*5)


extern void can0_reset(void);

static void _UpdataBleAdvData(uint8_t mac[6]);


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
	
	Printf("\tICCID = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
		g_Jt.property.iccid[0],g_Jt.property.iccid[1],
		g_Jt.property.iccid[2],g_Jt.property.iccid[3],
		g_Jt.property.iccid[4],g_Jt.property.iccid[5],
		g_Jt.property.iccid[6],g_Jt.property.iccid[7],
		g_Jt.property.iccid[8],g_Jt.property.iccid[9]);
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
	Printf("\tMAC = [%02X%02X%02X%02X%02X%02X]\n",
		g_Jt.bleproperty.BleMac[5],g_Jt.bleproperty.BleMac[4],
		g_Jt.bleproperty.BleMac[3],g_Jt.bleproperty.BleMac[2],
		g_Jt.bleproperty.BleMac[1],g_Jt.bleproperty.BleMac[0]);

	//Printf("\tBleName = %s\n", g_Jt.blecfgParam.BleName);
	
	PRINTF_BLEDEVCFG(BleAdvInterval);
	PRINTF_BLEDEVCFG(BleAdvPower);
	
	Printf("\tbleState=%X\n",g_Jt.bleState.bleConnectState);
	Printf("\tbleConnectMAC=%s\n",g_Jt.bleState.bleConnectMAC);
	
	

}


UTP_EVENT_RC JT808_cmd_getSimID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevProperty) - JT_DEV_HW_VER_SIZE - JT_DEV_FW_VER_SIZE;
	if (ev == UTP_CHANGED_AFTER )
	{
		uint8 ver[JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE];

		//ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½ï¿½Ýºï¿½propertyï¿½ï¿½ï¿½ï¿½Ä½á¹¹ï¿½ï¿½Æ¥ï¿½ä£¬ï¿½ï¿½Òªï¿½ï¿½ï¿½Â¸ï¿½ÖµHwVerï¿½ï¿½FwVer

		memcpy(ver, g_Jt.property.hwVer, pCmd->pExt->transferLen - headSize - 1);
		memset( g_Jt.property.hwVer, 0, JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE);

		if (ver[0] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(g_Jt.property.hwVer, &ver[1], ver[0]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 hwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(g_Jt.property.fwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 fwVer size(%d) error.\n", ver[offset]);
		}

		// ï¿½ï¿½Ð¡ï¿½ï¿½×ªï¿½ï¿½ Dt_convertToU32     SWAP32
		g_Jt.property.protocolVer = Dt_convertToU32( &g_Jt.property.protocolVer , DT_UINT32 );
		g_Jt.property.devClass = Dt_convertToU32( &g_Jt.property.devClass , DT_UINT32 );

		// Info
		PFL(DL_JT808,"SIM ID\r\n");
		PFL(DL_JT808,"ver:%02X\r\n",g_Jt.property.protocolVer);
		PFL(DL_JT808,"Calss:%d\r\n",g_Jt.property.devClass);
		PFL(DL_JT808,"devModel:%s\r\n",g_Jt.property.devModel);
		PFL(DL_JT808,"devID:%s\r\n",g_Jt.property.devId);
		PFL(DL_JT808,"ICCID:%s\r\n",g_Jt.property.iccid);
		PFL(DL_JT808,"hwVer:%s\r\n",g_Jt.property.hwVer);
		PFL(DL_JT808,"fwVer:%s\r\n",g_Jt.property.fwVer);

		gSendCanCmdCnt = 0 ;	// ï¿½ï¿½0
	}
	else if (ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
		//ï¿½ï¿½ï¿½Ã¶ï¿½È¡ï¿½ï¿½ï¿½ï¿½
		for (i = 0; (i + readParamOffset) < g_jtTlvOutMgr_8103.itemCount && i < 5; i++, p++)
		{
			paramIDs[i] = p->tag;
		}
		pCmd->pExt->transferLen = i * 2;
	}
	else if (ev == UTP_REQ_SUCCESS)	//ï¿½ï¿½È¡ï¿½É¹ï¿½
	{
		//UTP_FSM_WAIT_RSP == pUtp->state 
		readParamOffset += i;
		JtTlv8103_updateMirror( &pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1 );
		//JtTlv8103_updateStorage(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		if (readParamOffset < g_jtTlvOutMgr_8103.itemCount )//  g_jtTlvInMgr_8103.itemCount)
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
		}
		else //È«ï¿½ï¿½ï¿½ï¿½È¡ï¿½ï¿½ï¿?		
		{
			//i = 0;
			readParamOffset = 0;
			// ï¿½ï¿½ï¿½ï¿½ï¿½Ä¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½,ï¿½ï¿½ï¿½ÎªÔ¶ï¿½ï¿½ï¿½ï¿½ï¿½Ã£ï¿½ï¿½ò²»´ï¿½ï¿½ï¿½Ö?			if( JtTlv8103_getFactoryCofnig() != 0 )	// ï¿½Ç³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG );
			}
		}
		i = 0 ;
		gSendCanCmdCnt = 0x00 ;
	}
	else if (ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);	// Ò»ï¿½ï¿½Òªï¿½ï¿½È¡ï¿½ï¿½
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
		gGPRSConTimeCnt = GET_TICKS();		// ï¿½ï¿½Ê¼ï¿½ï¿½Ê±
	}
	else if ( ev == UTP_REQ_FAILED  )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
	if( ev == UTP_TX_START )
	{
		// ï¿½ï¿½ï¿½ï¿½Ç°ï¿½ï¿½ï¿½ï¿½Ê±ï¿½Ä±ï¿½
		pJt->blecfgParam.BleAdvInterval = Dt_convertToU32(&pJt->blecfgParam.BleAdvInterval,DT_UINT32);
	}
	else if( ev == UTP_TX_DONE )
	{
		// ï¿½ï¿½ï¿½Íºï¿½,ï¿½Ö¸ï¿½
		pJt->blecfgParam.BleAdvInterval = Dt_convertToU32(&pJt->blecfgParam.BleAdvInterval,DT_UINT32);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
		
		pJt->bleEnCtrl = Dt_convertToU16(&pJt->bleEnCtrl,DT_UINT16);
		if( pJt->bleEnCtrl & 0x01 )
		{
			//_JT808_DEBUGMSG("Disable BLE\r\n");
			pJt->bleEnCtrl = 0x00 ;
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
		}
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
	if( ev == UTP_TX_START )
	{
		// ï¿½ï¿½ï¿½ï¿½Ç°ï¿½ï¿½ï¿½ï¿½Ê±ï¿½Ä±ï¿½
		pJt->bleEnCtrl = Dt_convertToU16(&pJt->bleEnCtrl,DT_UINT16);
	}
	else if( ev == UTP_TX_DONE )
	{
		// ï¿½ï¿½ï¿½Íºï¿½,ï¿½Ö¸ï¿½
		pJt->bleEnCtrl = Dt_convertToU16(&pJt->bleEnCtrl,DT_UINT16);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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

		//ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½ï¿½Ýºï¿½propertyï¿½ï¿½ï¿½ï¿½Ä½á¹¹ï¿½ï¿½Æ¥ï¿½ä£¬ï¿½ï¿½Òªï¿½ï¿½ï¿½Â¸ï¿½ÖµHwVerï¿½ï¿½FwVer

		memcpy(ver, g_Jt.bleproperty.BlehwVer, pCmd->pExt->transferLen - headSize - 1);
		memset(g_Jt.bleproperty.BlehwVer, 0, JT_DEV_BLE_HW_VER_SIZE + JT_DEV_BLE_FW_VER_SIZE );

		if (ver[0] < JT_DEV_BLE_HW_VER_SIZE )
		{
			memcpy(g_Jt.bleproperty.BlehwVer, &ver[1], ver[0]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 BlehwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_BLE_FW_VER_SIZE )
		{
			memcpy(g_Jt.bleproperty.BlefwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			PFL(DL_JT808, "Jt808 BlefwVer size(%d) error.\n", ver[offset]);
		}
		g_Jt.bleproperty.BleType = Dt_convertToU16(&g_Jt.bleproperty.BleType,DT_UINT16);
		
		gSendCanCmdCnt = 0x00 ;
		PFL(DL_JT808,"BLE:%02X[%02X%02X%02X%02X%02X%02X]\r\n",
			g_Jt.bleproperty.BleType , 
			g_Jt.bleproperty.BleMac[0],g_Jt.bleproperty.BleMac[1],
			g_Jt.bleproperty.BleMac[2],g_Jt.bleproperty.BleMac[3],
			g_Jt.bleproperty.BleMac[4],g_Jt.bleproperty.BleMac[5]);
		PFL(DL_JT808,"BLE Ver:%s---%s\r\n",
			g_Jt.bleproperty.BlehwVer,g_Jt.bleproperty.BlefwVer );

		Ble_init( (uint8*)g_Jt.bleproperty.BleMac );

		_UpdataBleAdvData( (uint8*)g_Jt.bleproperty.BleMac );

	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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

	if (ev == UTP_REQ_SUCCESS)	//ï¿½ï¿½È¡ï¿½É¹ï¿½
	{
		g_BleCfgParam_mirror.BleAdvInterval = Dt_convertToU32(&g_BleCfgParam_mirror.BleAdvInterval,DT_UINT32);
		//g_BleCfgParam_mirror
		//_JT808_DEBUGMSG("BLE\r\nName:%s",
		//	g_BleCfgParam_mirror.BleName );
		//_JT808_DEBUGMSG("Inter:%d,Power:%d\r\n",
		//	g_BleCfgParam_mirror.BleAdvInterval,g_BleCfgParam_mirror.BleAdvPower );
		//_JT808_DEBUGMSG("Adv:%s\r\n",g_BleCfgParam_mirror.BleAdvData);
		// ï¿½é¿´ï¿½Ç·ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½BLE Name
		//JtDevBleCfgParam
		if( 0 != memcmp( &g_BleCfgParam_mirror , &g_Jt.blecfgParam ,sizeof(JtDevBleCfgParam) ))
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
		}
		gSendCanCmdCnt = 0x00;
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
	if (ev == UTP_REQ_SUCCESS)	//ï¿½ï¿½È¡ï¿½É¹ï¿½
	{
		pJt->updatefileinfo.Updatefilelength = Dt_convertToU32(&pJt->updatefileinfo.Updatefilelength,DT_UINT32);

		PFL(DL_JT808,"Update file Len:%x,Ver:%d\r\n",
			g_Jt.updatefileinfo.Updatefilelength,g_Jt.updatefileinfo.UpdatefileVersion );
		
		gSendCanCmdCnt = 0 ;

		if( g_Jt.updatefileinfo.Updatefilelength < APPLICATION_SIZE  )
		{
			// ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½Í¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			pJt->filecontentreq.fileOffset = 0 ;		
			pJt->filecontentreq.fileType = pJt->fileDesc.fileType ;
			Utp_DelaySendCmd( &g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT , 100 );		// ï¿½ï¿½Ò»ï¿½Î·ï¿½ï¿½ï¿½Îª 0
			PFL(DL_JT808,"Start Rev File Pack offset:%x,type:%d\r\n",
				pJt->filecontentreq.fileOffset , pJt->filecontentreq.fileType );
			// 
			EraseFirmwareAllArea();		// É¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		}
		else
		{
			PFL(DL_JT808,"Update File size >.\r\n ");
		}
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
	if (ev == UTP_TX_START) 	// ï¿½Ï±ï¿½
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS) //ï¿½ï¿½È¡ï¿½É¹ï¿½
	{
		gSendCanCmdCnt = 0 ;
		//ï¿½ï¿½ï¿½ï¿½×´Ì¬ï¿½É¹ï¿½

		// ï¿½ï¿½ï¿½ï¿½Ö®
		switch( pJt->setToOpState.OperationState )
		{
			case JT_STATE_SLEEP:
				pJt->opState = pJt->setToOpState.OperationState ;
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
				//gModuleSleepCnt = GET_TICKS();

				can0_sleep();	// CAN ÐÝÃß
				Utp_Reset(&g_JtUtp);
				Fsm_SetActiveFlag(AF_JT808, False);		// ÉèÖÃ×´Ì¬
				break;
			case JT_STATE_WAKEUP:
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
				//gModuleWakeupCnt = GET_TICKS();
				Fsm_SetActiveFlag(AF_JT808, True);
			
				break;
			default:break ;
		}
		PFL(DL_JT808,"ï¿½ï¿½ï¿½ï¿½OpState:%d[%d]\r\n",pJt->setToOpState.OperationState,pJt->opState );
	}
	else if ( ev == UTP_REQ_FAILED )	//ï¿½ï¿½È¡Ê§ï¿½Ü£ï¿½ï¿½ï¿½ï¿½Â¶ï¿½È¡
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
		Utp_DelaySendCmd(&g_JtUtp, JTCMD_SET_OP_STATE, 500);		// ï¿½ï¿½Ô¶ï¿½ï¿½ï¿½ï¿½×´Ì¬
	}
	return UTP_EVENT_RC_SUCCESS;
}

static void _SetOperationState(uint8_t Operation, uint8_t Parameter )
{
	g_Jt.setToOpState.OperationState = Operation ;
	g_Jt.setToOpState.StateParameter = Parameter ;
	Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
}
void JT808_CheckResetSerAddr(void)
{
	Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
}


// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½

void SetComModeSleep(void)
{
	if( g_Jt.opState != JT_STATE_SLEEP )
	{
		//Utp_Reset(&g_JtUtp);		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¸ï¿½ï¿?		_SetOperationState( JT_STATE_SLEEP , _OPERATION_SLEEP );
		_SetOperationState( JT_STATE_SLEEP , _OPERATION_SLEEP );
		PFL(DL_JT808,"JT808 Send Sleep Command\r\n");
	}
}

// Ä£ï¿½ï¿½ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½
Bool ComModeSleep(void)
{
	return ( g_Jt.opState == JT_STATE_SLEEP );
}
// Ä£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
Bool ComModeSleepIng(void)
{
	return ( ( g_Jt.opState != JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_SLEEP ) ) ;
}

// Ä£ï¿½ï¿½ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½
Bool ComModeWakeup(void)
{
	return ( g_Jt.opState == JT_STATE_WAKEUP );
}


// Ä£ï¿½ï¿½ï¿½ï¿½ï¿½Ú»ï¿½ï¿½ï¿½
Bool ComModeWakeupIng(void)
{
	return ( ( g_Jt.opState == JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_WAKEUP ) ) ;
}

// ï¿½ï¿½ï¿½ï¿½Ä£ï¿½é»½ï¿½ï¿½
Bool ComIRQWakeup(void)
{
	extern FlagStatus modul_receive_flag;
	return modul_receive_flag == 1 ;
}

void SetComModeWakeup(void)
{
	if( g_Jt.opState == JT_STATE_SLEEP )
	{
		//Utp_Reset(&g_JtUtp);		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¸ï¿½ï¿?		_SetOperationState( JT_STATE_WAKEUP , _OPERATION_WKUP );
		_SetOperationState( JT_STATE_WAKEUP , _OPERATION_WKUP );
		PFL(DL_JT808,"JT808 Send Wakup Command\r\n");
	}
}



void JT808_switchState(JT808* pJt, JT_state newState)
{
	if( newState > JT_STATE_OPERATION ) return ;
	
	if (pJt->opState == newState) return;
	
	switch (newState)
	{
		case JT_STATE_INIT:
			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			// ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				_SetOperationState( JT_STATE_PREOPERATION,_OPERATION_PRE);
			}
			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì?			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			Fsm_SetActiveFlag(AF_JT808, True );
			break;
		case JT_STATE_SLEEP:
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;

			// ï¿½Ø±ï¿½CAN ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			can0_sleep();
			Utp_Reset(&g_JtUtp);
			Fsm_SetActiveFlag(AF_JT808, False);		// ÉèÖÃ×´Ì¬
			break;
		case JT_STATE_WAKEUP:
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				_SetOperationState(JT_STATE_PREOPERATION,_OPERATION_PRE );
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			Fsm_SetActiveFlag(AF_JT808, True );
			
			break;
		case JT_STATE_PREOPERATION:
			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			// ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);// 0x11
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_ID); //0x30
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_CFG); //0x31
				//Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN); // 0x33			
				//Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);//0x16
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			gGPSTimeCnt = GET_TICKS();
			gGPRSConTimeCnt = GET_TICKS();	
			Fsm_SetActiveFlag(AF_JT808, True );
			break;
		case JT_STATE_OPERATION:
			gGPSTimeCnt = GET_TICKS();
			gGPRSConTimeCnt = GET_TICKS();
			Fsm_SetActiveFlag(AF_JT808, True );
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
		// Í¨ï¿½ï¿½Ä£ï¿½ï¿½×´Ì¬ï¿½ï¿½ ï¿½ï¿½ï¿½Ø±ï¿½ï¿½ï¿½×´Ì¬ï¿½ï¿½Ò»ï¿½ï¿½Ê±
		if( pJt->opState != pJt->setToOpState.OperationState )
		{
			PFL(DL_JT808,"Change OPState[%d-%d]\r\n",pJt->opState ,pJt->setToOpState.OperationState );
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				pJt->opState = JT_STATE_INIT;		// ï¿½ï¿½ï¿½Â¿ï¿½Ê¼
				_SetOperationState(JT_STATE_PREOPERATION,_OPERATION_PRE );
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


/*
pCmd->pExt->transferData = (uint8*)pData;
pCmd->pExt->transferLen = frameLen;

*/
UTP_EVENT_RC JT808_event_devStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_BEFORE ) // ï¿½ä»¯Ö®Ç°
	{
		uint16_t cnt_state = *(uint16_t*)pCmd->pExt->transferData ;
		PFL(DL_JT808,"Cur Cnt:%x\r\n", cnt_state );
		if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT ) // ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			if( ! ( cnt_state & _NETWORK_CONNECTION_BIT ) )
			{
				gGPRSConTimeCnt = GET_TICKS();
			}
		}
		else if( cnt_state & _NETWORK_CONNECTION_BIT )
		{
			uint32_t con_ms = 0;
			con_ms = GET_TICKS() - gGPRSConTimeCnt ;
			gGPRSConCnt = GET_TICKS();
			PFL(DL_JT808,"Newtwork Connect time:%dms\r\n", con_ms );
		}

		if( g_Jt.devState.cnt & _GPS_FIXE_BIT )
		{
			if( ! ( cnt_state & _GPS_FIXE_BIT ) )
			{
				gGPSTimeCnt = GET_TICKS();
			}
		}
		else if ( cnt_state & _GPS_FIXE_BIT )
		{
			uint32_t gps_ms = 0;
			gGPSFixCnt = GET_TICKS();
			gps_ms = GET_TICKS() - gGPSTimeCnt;
			PFL(DL_JT808,"GPS Fixe Time:%dms\r\n",gps_ms );
		}
	}
	else if( ev == UTP_CHANGED_AFTER )
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
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );// ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½
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
		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		//smsRevTime
		g_Jt.smsContext.smsText.smsRevTime = Dt_convertToU32( &g_Jt.smsContext.smsText.smsRevTime ,DT_UINT32 );
		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		PFL(DL_JT808,"SMS:%d,SMSCout:%d,Len:%d,Time:%d\r\n",
			g_Jt.smsContext.smsExist,g_Jt.smsContext.smsCount,
			g_Jt.smsContext.smsText.smsLen,g_Jt.smsContext.smsText.smsRevTime );
		// ï¿½ï¿½ï¿½Ð¶ï¿½ï¿½ï¿½
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
	 	PFL(DL_JT808, "8900 Rx[%d]:",pCmd->pExt->transferLen);
		DUMP_BYTE_LEVEL(DL_JT808, pCmd->pExt->transferData, pCmd->pExt->transferLen );
		PFL(DL_JT808, "\n");	 	
	  	JtTlv8900_proc(pCmd->pExt->transferData, pCmd->pExt->transferLen);
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

/*
	ï¿½Ç·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ -- ï¿½Õ¼ï¿½      and ï¿½ï¿½ï¿?> 60% and ï¿½Ç·ï¿½ï¿½ï¿½ï¿½ï¿½
*/
static Bool _CheckFirmwareUpdate(void)
{
	// Ã»ÓÐ·Åµç & µç³ØµçÁ¿²»ÒªµÍ
	if( g_pdoInfo.isLowPow ) return false;
	//if( g_pdoInfo.isRemoteAccOn ) return false ;	
	return true ;
}


UTP_EVENT_RC JT808_event_rcvFileData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	//PFL(DL_JT808,"rvcFileData :%d\r\n", ev );
	 if (ev == UTP_REQ_SUCCESS)
	 {
	 	switch( pJt->fileDesc.fileType )
	 	{
			case 0x01 :
				PFL(DL_JT808,"C7 Update File:%d\r\n",pJt->fileDesc.fileVerDesc);
				if( _CheckFirmwareUpdate() )
				{
					Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);
					PFL(DL_JT808,"Start Get Update File Info\r\n");
				}
				break ;
			case 0x02 :
				//
				PFL(DL_JT808,"Key Update:%d\r\n",pJt->fileDesc.fileVerDesc);
				break ;
			default :
				PFL(DL_JT808,"Unkonw Update :%d:%d\r\n",pJt->fileDesc.fileType,pJt->fileDesc.fileVerDesc);
				break ;
	 	}
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
		//pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//Ä¬ï¿½ÏµÈ´ï¿½ï¿½ï¿½Ó¦ï¿½ï¿½Ê±ï¿½ï¿½Îª1ï¿½ï¿½
		//pUtp->maxTxCount = 3;		//Ä¬ï¿½Ïµï¿½ï¿½Ø·ï¿½ï¿½ï¿½ï¿½ï¿½Îª3

		g_JtUtp.waitRspMs = 1000 * 10 ;	// 10s
	
		pCmd->pExt->transferLen = g_txlen;
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		g_txlen = 0;
		gSendCanCmdCnt = 0x00;
		//int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
		//JtTlv0900_updateMirror( g_txBuf , len );
		JtTlv0900_UpdateMirror();	// ï¿½ï¿½ï¿½ï¿½
		PFL(DL_JT808,"0900 UpdataMirror\r\n");
	}
	else if ( ev == UTP_REQ_FAILED )
	{
		PFL_WARNING("update Bat failed failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reboot\r\n");
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_SEND_TO_SVR,gSendCanCmdCnt);
			
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getFileContent(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_TX_START )	// ï¿½ï¿½ï¿½Íµï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½Ò»ï¿½ï¿½
	{
		//ï¿½ï¿½ï¿½ï¿½Ö®Ç°ï¿½ï¿½ï¿½ï¿½Ê±--ï¿½ï¿½É´ï¿½ï¿½Ä£Ê½ï¿½Ô±ã·¢ï¿½ï¿½
		pJt->filecontentreq.fileOffset = Dt_convertToU32( &pJt->filecontentreq.fileOffset , DT_UINT32 ); 		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ÒªÈ¡ï¿½ï¿½
	}
	else if( ev == UTP_TX_DONE )
	{
		//ï¿½ï¿½ï¿½ï¿½Ö®ï¿½ó£¬»Ø¸ï¿½Ö®
		pJt->filecontentreq.fileOffset = Dt_convertToU32( &pJt->filecontentreq.fileOffset , DT_UINT32 );
	}
	else if( ev == UTP_GET_RSP )		// ï¿½ï¿½Ó¦ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
	{
		//
	}
	else if(ev == UTP_REQ_SUCCESS )
	{
		pJt->filecontentrsq.fileOffset = Dt_convertToU32( &pJt->filecontentrsq.fileOffset , DT_UINT32);
		if( pJt->filecontentreq.fileOffset == pJt->filecontentrsq.fileOffset )
		{
			// Ð´Flash
			WriteFirmware( pJt->filecontentrsq.fileOffset,pJt->filecontentrsq.fileData , 128 );
			// test
			//{
			//	uint8 buff[128];
			//	ReadFirmware( pJt->filecontentrsq.fileOffset , buff , 128 );
			//	if( 0 != memcmp( pJt->filecontentrsq.fileData , buff , sizeof(buff)) )
			//	{
			//		PFL_WARNING("Write File Error:%x\r\n",pJt->filecontentrsq.fileOffset);
			//	}
			//}
			
			pJt->filecontentreq.fileOffset += 128 ;			
			if( pJt->filecontentreq.fileOffset >= pJt->updatefileinfo.Updatefilelength )
			{
				if( CheckFirmware( pJt->updatefileinfo.Updatefilelength ) )
				{
					PFL(DL_JT808,"Check File OK ,Entern bootloader\r\n");
					NVIC_SystemReset();
				}
				else
				{
					EraseFirmwareInfoArea();
					PFL(DL_JT808,"Check File Error ,Delete File\r\n");
				}
			}
			else
			{
				Utp_DelaySendCmd( &g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT , 50 );
				//Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT );		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
				PFL(DL_JT808,"Rev File Next Pack :%d\r\n",pJt->filecontentreq.fileOffset );
			}
		}
		gSendCanCmdCnt = 0 ;

		Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ã·¢

		Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);	// ·ÀÖ¹ËüÐÝÃß
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		gSendCanCmdCnt = 0 ;
	}
	else if ( ev == UTP_REQ_FAILED )
	{
		PFL_WARNING("update file failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			can0_reset();
			PFL_WARNING("Resend Cmd Timeout,CAN reboot, stop update file \r\n");
			// ï¿½ï¿½ï¿?			EraseFirmwareInfoArea();
		}
		else
		{
			PFL_WARNING("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_SEND_TO_SVR,gSendCanCmdCnt);
			Utp_DelaySendCmd( &g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT , 30 );
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_event_setLocationExtras(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC JT808_event_rcvBleData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{	
	// Ó¦ï¿½Ã´ï¿½ï¿½ï¿½ UTP_CHANGED_AFTER ï¿½Â¼ï¿½
	if( ev == UTP_CHANGED_AFTER )	// ï¿½Õµï¿½ï¿½ï¿½ï¿½ï¿½
	{
		//
	}
	else if (ev == UTP_GET_RSP) 	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
	{
		Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð´ï¿½ï¿½ pCmd->pExt->transferData		& pCmd->pExt->transferLenï¿½ï¿½ï¿½ï¿½
		// test
		//uint8_t test_buf[] = {'0','1','2','3','4','5','6','7','8','9','A'};
		//memcpy(pCmd->pExt->transferData,test_buf,sizeof(test_buf));
		//pCmd->pExt->transferLen = sizeof(test_buf);
		
		uint8 rspLen = 0;
		pCmd->pExt->transferData = Ble_ReqProc(pCmd->pStorage, gCurRevLen, &rspLen);
		pCmd->pExt->transferLen = rspLen;
		if (pCmd->pExt->transferData == Null)
		{
			return UTP_EVENT_RC_FAILED;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//
UTP_EVENT_RC JT808_event_bleStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_BEFORE )
	{
		//
	}
	else if ( ev == UTP_CHANGED_AFTER )
	{
		if( g_Jt.bleState.bleConnectState & _BLE_CONNECT_BIT )
		{
			Ble_Logout();	// Çå³ýÖ®
			PFL(DL_JT808,"BLE Connect[%02X:%02X:%02X:%02X:%02X:%02X]\r\n",
				g_Jt.bleState.bleConnectMAC[0],g_Jt.bleState.bleConnectMAC[1],
				g_Jt.bleState.bleConnectMAC[2],g_Jt.bleState.bleConnectMAC[3],
				g_Jt.bleState.bleConnectMAC[4],g_Jt.bleState.bleConnectMAC[5]);
		}
		else
		{
			PFL(DL_JT808,"Ble Disconnect\r\n");
			Ble_Logout();	// Çå³ýÖ®
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_bleAuthChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}



//ï¿½ï¿½ï¿½Ðµï¿½ï¿½ï¿½ï¿½î£¬ï¿½Ð´ï¿½ï¿½ï¿½ï¿½Â¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ã¸Ã»Øµï¿½ï¿½ï¿½ï¿½ï¿?
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
		//ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½Ðµï¿½ï¿½è±¸ï¿½ï¿½Ï¢ï¿½ï¿½ï¿½Ð»ï¿½ï¿½ï¿½OPERATION×´Ì¬
		const UtpCmd* pCmd = (UtpCmd*)param1;
		if (pCmd->cmd == JTCMD_CMD_GET_BLE_CFG/*JTCMD_CMD_GET_FILE_INFO*/)
		{
			_SetOperationState(JT_STATE_OPERATION,_OPERATION_OPE );
		}
	}
}

void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_RUN)
	{
		// ï¿½ï¿½ï¿½ï¿½JtTlv9000ï¿½ï¿½ï¿½ï¿½
		JtTlv0900_updateStorage();	//
		
		if (Utp_isIdle(&g_JtUtp) && (g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT) )	// ï¿½ï¿½ï¿½ï¿½ & ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½s
		{
			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
			if (len) 
			{
				g_txlen = len;		// È«ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
				PFL(DL_JT808,"Send Bat Info:%d\r\n",len );
				// save 
				JtTlv0900_Cache( g_txBuf , len );
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

	//ï¿½ï¿½ï¿½ò²»¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ðµï¿½ï¿½ï¿½ï¿½ï¿½
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




#define	_CAN_TX_TIMEOUT_MS		(100)
extern can_trasnmit_message_struct transmit_message;
extern can_receive_message_struct  receive_message;
extern FlagStatus modul_receive_flag;
//xx ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ýµï¿½ï¿½ï¿½ï¿½ï¿½ Ò»ï¿½ï¿½ï¿½ï¿½à·¢ï¿½ï¿?ï¿½ï¿½ï¿½Ö½ï¿½ ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½Ð¶ï¿½ï¿½Ç·ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿?
int JT808_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmdÎªCANÐ­ï¿½ï¿½ï¿½PF
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
		//PFL(DL_CAN, "CANTx:");
		//PFL(DL_CAN, "%08x ",transmit_message.tx_efid);
		//DUMP_BYTE_LEVEL(DL_CAN,&transmit_message.tx_data,transmit_message.tx_dlen);
		//PFL(DL_CAN, "\n");
		//rt_thread_mdelay(20);
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


//xx ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½

void JT808_rxDataProc(const uint8_t* pData, int len)
{
	//Utp_RxData(&g_JtUtp, pData, len);
	CirBuffPush( &g_JtUtp.rxBuffCirBuff, pData ,len );
}



void JT808_timerProc()
{
	if( modul_receive_flag == SET )
	{
		modul_receive_flag = RESET;
		//PFL(DL_CAN, "CANRx:");
		//PFL(DL_CAN, "%08x ",receive_message.rx_efid);
		//DUMP_BYTE_LEVEL(DL_CAN,&receive_message.rx_data,receive_message.rx_dlen);
		//PFL(DL_CAN, "\n");
		/*
		if(0x00001020 == receive_message.rx_efid)
		{
			JT808_rxDataProc( receive_message.rx_data , receive_message.rx_dlen );
			gCanbusRevTimeMS = GET_TICKS();
		}
		*/
		//JT808_rxDataProc( receive_message.rx_data , receive_message.rx_dlen );
		gCanbusRevTimeMS = GET_TICKS();
		//PFL(DL_JT808,"CAN Rev len:%d,ID:%x:%d-%d\n",
		//	receive_message.rx_dlen,receive_message.rx_efid,g_JtUtp.rxBuffCirBuff.miHead,g_JtUtp.rxBuffCirBuff.miTail );
		g_pdoInfo.isCANOk = 1;
	}
	else
	{
		if( GET_TICKS() - gCanbusRevTimeMS > _CAN_BUS_REV_TIMEOUT_MS )
		{
			PFL_WARNING("CAN Rev Timeout.reset can\r\n");
			can0_reset();
			gCanbusRevTimeMS = GET_TICKS();
			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì?			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			g_pdoInfo.isCANOk = 0;
		}
	}
}

#if 0
void JT808_run()
{
	JT808_timerProc();
	JT808_fsm(MSG_RUN, 0, 0);
	#ifdef DGT_CONFIG	
	WDOG_Feed();
	#endif
	Utp_Run(&g_JtUtp);
}
#else

static void JT808_Work(void)
{
	JT808_timerProc();
	JT808_fsm(MSG_RUN, 0, 0);
	#ifdef DGT_CONFIG	
	WDOG_Feed();
	#endif
	Utp_Run(&g_JtUtp);
}




#define		_SLEEP_TIMEOUT_MS			(15*1000)			// 15*1000 Ä£¿éÐÝÃß
#define		_WAKEUP_TIMEOUT_MS			(30*1000)			// 15*1000	Ä£¿é»½ÐÑ³¬Ê±


extern JT808ExtStatus gJT808ExtStatus ;


void JT808_run(void)
{
	if( ComModeSleep() )	// Ä£¿éÒÑ¾­ÐÝÃß---¼ì²âÊÇ·ñÐèÒª»½ÐÑ´¦Àí
	{
		if( ComModeWakeupIng() )	// ÕýÔÚ±»»½ÐÑ
		{
			if( GET_TICKS() - gModuleWakupIngTimeoutCnt > _WAKEUP_TIMEOUT_MS )
			{
				//³¬Ê±Ò²Ã»°ì·¨£¬Ö»ÄÜÖØÆôCAN
				can0_reset();
				gModuleWakupIngTimeoutCnt = GET_TICKS();		// ÖØÐÂ¿ªÊ¼
				PFL_WARNING("GPRS/GPS Wakeup Timeout\r\n");
			}
			JT808_Work();
		}
		else
		{
			if( !WorkMode_Sleep() && _JT808_EXT_SLEEP != gJT808ExtStatus )	// ÍâÎ§»½ÐÑ
			{
				gModuleWakupIngTimeoutCnt = GET_TICKS();	// ¼ÆÊ±

				can0_wakeup();
				Utp_Reset(&g_JtUtp);
				SetComModeWakeup();
				PFL(DL_JT808,"GPRS/GPS Wakeup[%d]...\r\n",gJT808ExtStatus );

				JT808_Work();
			}
		}
	}
	else					// Èç¹ûÎ´ÐÝÃß,Ôò¼ì²âÊÇ·ñÐèÒªÐÝÃß
	{
		if( ComModeSleepIng() )		// ÕýÔÚ½øÈëÐÝÃß
		{
			//¼ì²â½øÐÝÃßÊÇ·ñ³¬Ê±
			if( GET_TICKS() - gModuleSleepIngTimeoutCnt > _SLEEP_TIMEOUT_MS )
			{
				//³¬Ê±Ò²Ã»°ì·¨£¬Ö»ÄÜÖØÆôCAN
				can0_reset();
				Utp_Reset(&g_JtUtp);
				SetComModeSleep();	// ×¼±¸½øÈëÐÝÃß
				gModuleSleepIngTimeoutCnt = GET_TICKS();
				PFL_WARNING("GPRS/GPS Sleep Timeout\r\n");
			}
		}
		else 					// Ã»ÓÐÐÝÃß¼ì²âÊÇ·ñÐèÒª½øÐÝÃß
		{
			if( _JT808_EXT_SLEEP == gJT808ExtStatus )
			{
				SetComModeSleep();	// ×¼±¸½øÈëÐÝÃß
				gModuleSleepIngTimeoutCnt = GET_TICKS();		// ¼ÆÊ±
			}
			else if(_JT808_EXT_BRIEF_WAKUP == gJT808ExtStatus )	// ÁÙÊ±»½ÐÑ ¶¨Î»30sºó,»òÕßÆô¶¯5·ÖÖÓºó½øÐÝÃß
			{
				// ³¬Ê±15sºóÐÝÃß
				//gModuleWakupIngTimeoutCnt
				if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT &&
					g_Jt.devState.cnt & _GPS_FIXE_BIT &&
					GET_TICKS() - gModuleWakupIngTimeoutCnt > 3*60*1000  )
				{
					SetComModeSleep();	// ×¼±¸½øÈëÐÝÃß
					gModuleSleepIngTimeoutCnt = GET_TICKS();		// ¼ÆÊ±
					PFL(DL_JT808,"JT808 BRIEF Wakeup GPRS&GPS fix,Sleep\n");

					gJT808ExtStatus = _JT808_EXT_SLEEP;	// Çå³ýÖ®
				}
				else if( GET_TICKS() - gModuleWakupIngTimeoutCnt > 5*60*1000 )
				{
					SetComModeSleep();	// ×¼±¸½øÈëÐÝÃß
					gModuleSleepIngTimeoutCnt = GET_TICKS();		// ¼ÆÊ±
					PFL(DL_JT808,"JT808 BRIEF Wakeup timeout > 5min,Sleep\n");

					gJT808ExtStatus = _JT808_EXT_SLEEP;	// Çå³ýÖ®
				}
			}
		}
		//else // Ã»ÓÐ×¼±¸½øÈëÐÝÃß,Ôò¼ì²âÊÇ·ñÐèÒª½øÐÝÃß
		JT808_Work();
	}
}

#endif //

Bool JT808_wakeup(void)
{
	return True;
}

Bool JT808_sleep(void)
{
	// ÐÝÃßÖ®
	SetComModeSleep();	// ×¼±¸½øÈëÐÝÃß
	gModuleSleepIngTimeoutCnt = GET_TICKS();		// ¼ÆÊ±
	return True;
}

void JT808_start(void)
{
	//ï¿½ï¿½ï¿½ï¿½Ó²ï¿½ï¿½ï¿½ï¿½Ê¹ï¿½ï¿½ï¿½Ð¶ï¿½
	JT808_switchState(g_pJt, JT_STATE_INIT);

	gCanbusRevTimeMS = GET_TICKS();			
}

/*
0E09494D542D43372D343544374246
0319C0
03020105
03031218
09FF33300015830FF008
11073923CF407316429A5C417E7DC49A8314
*/
static void _UpdataBleAdvData(uint8_t mac[6])
{
	uint8_t i = 0 ;
	uint8_t _adv[31+31] ={0x00};
	uint8_t _adv_type[] = {0x03,0x19,0xC0,0x03,0x02,0x01,0x05,0x03,0x03,0x12,0x18};
	uint8_t _adv_uuid[] = {0x11,0x07,0x39,0x23,0xCF,0x40,0x73,0x16,0x42,0x9A,0x5C,0x41,0x7E,0x7D,0xC4,0x9A,0x83,0x14};
	memset( _adv , 0 , sizeof(_adv) );
	// Ìí¼ÓÃû×Ö
	// Name
	{
		uint8_t j = i , len = 0 ;		
		_adv[i++] = 0;// ï¿½ï¿½ï¿½ï¿½ 1 + sizeof(_BLE_NAME) - 1;
		_adv[i++] = 0x09;	// name
		len = sprintf( (char*)_adv + i ,_BLE_NAME , _DEV_MODEL, mac[3],mac[4],mac[5]);
		_adv[j] = 1 + len ;
		i += len ;
	}
	//
	memcpy( _adv + i , _adv_type , sizeof(_adv_type) );
	i += sizeof(_adv_type);
	// MAC µØÖ·
	{
		_adv[i++] = 1 + sizeof(_BLE_COMPY) - 1 + 6;
		_adv[i++] = 0xFF;
		//_adv[i++] = 
		_adv[i++] = mac[5];
		_adv[i++] = mac[4];
		_adv[i++] = mac[3];
		_adv[i++] = mac[2];
		_adv[i++] = mac[1];
		_adv[i++] = mac[0];

		memcpy( _adv + i , _BLE_COMPY , sizeof(_BLE_COMPY) - 1 );
		i += sizeof(_BLE_COMPY) - 1;
	}
	// UUID
	memcpy( _adv + i , _adv_uuid , sizeof(_adv_uuid) );
	i += sizeof(_adv_uuid);
	
	sprintf( (char*)g_Jt.blecfgParam.BleName ,_BLE_NAME , _DEV_MODEL , mac[3],mac[4],mac[5]);
	memcpy( g_Jt.blecfgParam.BleAdvData , _adv , sizeof(_adv));
}



void JT808_init()
{
	
/************************************************
*ï¿½ï¿½ï¿½Â±ï¿½ï¿½ï¿½ï¿½ï¿½Ð­ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð­ï¿½ï¿½Ê¹ï¿½ï¿?************************************************/
	
	#define JT_CMD_SIZE 23
	static uint8_t g_protocolVer = 1;	//ï¿½ï¿½ï¿½ï¿½Ð­ï¿½ï¿½æ±¾ï¿½ï¿?	//static uint8_t g_updatefiletype = 1; // ï¿½Ð¿ï¿½
	static uint8_t g_rxBuf[192];	
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//Î»ï¿½ï¿½Ô½ï¿½ï¿½Ç°ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¼ï¿½Ô½ï¿½ï¿½
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
		{&g_JtCmdEx[10],UTP_READ , JTCMD_CMD_GET_FILE_INFO , "GetFileInfo",(uint8_t*)&g_Jt.updatefileinfo , sizeof(UpdateFileInfo),(uint8_t*)&g_Jt.fileDesc.fileType,sizeof(uint8),(UtpEventFn)JT808_cmd_getFileInfo },
		{&g_JtCmdEx[11],UTP_WRITE , JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.setToOpState, sizeof(JT_SetOperationState) , Null , 0 , (UtpEventFn)JT808_cmd_setToOpState},
		{&g_JtCmdEx[12],UTP_WRITE, JTCMD_CMD_SEND_TO_SVR, "SendDataToSvr", (uint8_t*)g_txBuf, sizeof(g_txBuf), Null, 0, (UtpEventFn)JT808_event_sendSvrData},
		{&g_JtCmdEx[13],UTP_READ,JTCMD_CMD_GET_SMS,"GetSMSContext",(uint8_t*)&g_Jt.smsContext,sizeof(GetSMSContext),Null,0,(UtpEventFn)JT808_event_getSMSContext},
		{&g_JtCmdEx[14],UTP_READ , JTCMD_CMD_GET_FILE_CONTENT, "GetFileContent"	, (uint8_t*)&g_Jt.filecontentrsq, sizeof(FileContentRsq), (uint8_t*)&g_Jt.filecontentreq, sizeof(FileContentReq),(UtpEventFn)JT808_cmd_getFileContent},
		{&g_JtCmdEx[15],UTP_WRITE, JTCMD_CMD_SET_LOCATION_EXTRAS, "SetLocationExtras", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_setLocationExtras},

		// EVENT 
		// ï¿½ï¿½ï¿½ï¿½ & GPS
		{&g_JtCmdEx[16],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},
		{&g_JtCmdEx[17],UTP_EVENT, JTCMD_EVENT_DEV_STATE_LOCATION, "LocationChanged", (uint8_t*)& g_Jt.locatData, sizeof(Jt_LocationData), Null, 0, (UtpEventFn)JT808_event_LocationChanged},
		{&g_JtCmdEx[18],UTP_EVENT, JTCMD_EVT_RCV_SVR_DATA, "RcvSvrData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvSvrData},
		{&g_JtCmdEx[19],UTP_EVENT, JTCMD_EVT_RCV_FILE_DATA, "RcvFileData", (uint8_t*)&g_Jt.fileDesc, sizeof(FileVersionDesc), Null, 0, (UtpEventFn)JT808_event_rcvFileData},
		// ï¿½ï¿½ï¿½ï¿½
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
	g_Jt.opState = JT_STATE_UNKNOWN;	//ï¿½ï¿½Ê¼ï¿½ï¿½ÎªÒ»ï¿½ï¿½UNKNOWNÖµ

	{
		_UpdataBleAdvData( (uint8_t*)g_Jt.bleproperty.BleMac );
		g_Jt.blecfgParam.BleAdvInterval = 30;	
		g_Jt.bleEnCtrl = 0x01 ;
	}

	Utp_Init(&g_JtUtp, &g_cfg, &g_jtFrameCfg);

	JtTlv8900_init();
	JtTlv0900_init();
	JtTlv8103_init();
	
	Fsm_SetActiveFlag(AF_JT808, True);			// 
}



// JT808 Task
//void Jt808Task_thread_entry(void* pReader)
//{
//	static uint8_t last_module_st = 0 ;	// Ä¬ï¿½ï¿½Ä£ï¿½é²»ï¿½ï¿½ï¿½ï¿½
//	uint8_t cur_module_st = 0 ;
//	while (1)
//	{
		// ï¿½È´ï¿½×´Ì¬
//		JT808_run();
//		rt_thread_mdelay(10);
//	}
//}

//void Jt808TaskInit(void)
//{
//	rt_thread_t jt808_task_tid = rt_thread_create("JT808",/* ï¿½ß³ï¿½ï¿½ï¿½ï¿½ï¿½ */
//		Jt808Task_thread_entry, NULL,
//		512, 3, 10); //
//	JT808_start();
//	rt_thread_startup(jt808_task_tid);
//}

#endif //#ifdef CANBUS_MODE_JT808_ENABLE


