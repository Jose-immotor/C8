
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
#include "Dbg.h"
#include "drv_adc.h"


/*
{
	// MCU��С��ģʽ
	// CAN Э����ô��ģʽ
	// NFC Э����ô��ģʽ
	// Rs485���ô��ģʽ����
	uint32_t no = 0x12345678 ;
	uint8_t *pno = (uint8_t*)&no;
	Printf("no:0x%08X,0x%02X,0x%02X\n",no,no >> 24,pno[0]); //no:0x12345678,0x12,0x78
}
*/


#ifdef CANBUS_MODE_JT808_ENABLE

JT808 g_Jt;
JT808* g_pJt = &g_Jt;
static Utp g_JtUtp;
//static uint32_t g_hbIntervalMs = 2000;	//MCU�������?��λMS
static uint8_t g_txBuf[255];	//100
static uint16_t g_txlen = 0;
uint8_t g_hbdata[4] = {0x00, 0x00, 0x07, 0xD0};

// Jose add
static JtDevBleCfgParam g_BleCfgParam_mirror ;
static uint8_t gSendCanCmdCnt = 0 ;	// ���ͼ���---����ʧ�ܶ��?������֮...
static uint32_t gCanbusRevTimeMS = 0;
#define			_RESEND_CMD_COUNT		(2)	// �ط�3

// GPSʱ��---���Լ�¼ GPS��λ��ʱʱ��
static uint32 	gGPSTimeCnt ;		// 
static uint32 	gGPSFixCnt;			// GPS ��ʱʱ��
// GPRSʱ��---���Լ�¼GPRS�ӷ��͵�����ʱ��
//static uint32 	gGPRSSendTimeCnt;
static uint32 	gGPRSConTimeCnt ;		// ������ʱʱ��
static uint32	gGPRSConCnt;			// GPRS ����ʱ��


// sleep 
static uint32  gModuleSleepIngTimeoutCnt ;		// ģ����������߳�ʱ��ʱ
//static uint32  gModuleSleepCnt;					// ģ�����߼�ʱ
// wakeup 
static uint32  gModuleWakupIngTimeoutCnt;		// ģ������wakeup��ʱ��ʱ
//static uint32  gModuleWakeupCnt;				// ģ�黽�Ѽ�ʱ

Bool gJT808UpdateFirmware = False ;	// ���֮

Bool gJT808Disconnect = False ;		// JT808�Ƿ�Ͽ�,����1����δ�յ�CAN����,����Ϊ��Ͽ�

uint16_t gCurRevLen = 0 ;

// Beacon����
static uint32 gBeaconTick = 0;
static Bool gBeaconUpdate = False ;
static uint8 gBleAdvCntextCnt = 0;
static Beacon gBeaconCntext[10];	//�����ԭʼ����--С�˸�ʽ
static Bool gLocationFlag = False ;

// GPS 1sһ��,����2sһ��
#define			_CAN_BUS_REV_TIMEOUT_MS			(1000*5)

void JT808CAN_Sleep(void);

extern void can0_reset(void);

static void _UpdataBleAdvData(uint8_t mac[6]);


#define		_MODULE_SET_GPRS_STATE		do{g_Jt.devState.cnt |= _NETWORK_CONNECTION_BIT ;}while(0)	
#define		_MODULE_CLR_GPRS_STATE		do{g_Jt.devState.cnt &= ~_NETWORK_CONNECTION_BIT ;}while(0)
#define		_MODULE_SET_GPS_STATE		do{g_Jt.devState.cnt |= _GPS_FIXE_BIT ;}while(0)
#define		_MODULE_CLR_GPS_STATE		do{g_Jt.devState.cnt &= ~_GPS_FIXE_BIT ;}while(0)



void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2);

static JT808fsmFn JT808_findFsm(JT_state state);

static void _SetOperationState(uint8_t Operation, uint8_t Parameter );
void CtrlBLE( uint16_t ctrl );
void JT808_start(void);

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
		g_Jt.bleproperty.BleMac[0],g_Jt.bleproperty.BleMac[1],
		g_Jt.bleproperty.BleMac[2],g_Jt.bleproperty.BleMac[3],
		g_Jt.bleproperty.BleMac[4],g_Jt.bleproperty.BleMac[5]);

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

		//���յ����ݺ�property����Ľṹ��ƥ�䣬��Ҫ���¸�ֵHwVer��FwVer

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

		// ��С��ת�� Dt_convertToU32     SWAP32
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

		gSendCanCmdCnt = 0 ;	// ��0
	}
	else if (ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
		//���ö�ȡ����
		for (i = 0; (i + readParamOffset) < g_jtTlvOutMgr_8103.itemCount && i < 5; i++, p++)
		{
			paramIDs[i] = p->tag;
		}
		pCmd->pExt->transferLen = i * 2;
	}
	else if (ev == UTP_REQ_SUCCESS)	//��ȡ�ɹ�
	{
		//UTP_FSM_WAIT_RSP == pUtp->state 
		readParamOffset += i;
		JtTlv8103_updateMirror( &pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1 );
		//JtTlv8103_updateStorage(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		if (readParamOffset < g_jtTlvOutMgr_8103.itemCount )//  g_jtTlvInMgr_8103.itemCount)
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
		}
		else //ȫ����ȡ���?		
		{
			//i = 0;
			readParamOffset = 0;
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG );
		}
		i = 0 ;
		gSendCanCmdCnt = 0x00 ;
	}
	else if (ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);	// һ��Ҫ��ȡ��
	}

	return UTP_EVENT_RC_SUCCESS;
}


// Joe add 2020/9/17


UTP_EVENT_RC JT808_cmd_setSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_TX_START )
	{
		uint8_t tlvCount = 0 ;

		if( JtTlv8103_getFactoryCofnig() != 0 )	// �ǳ�������
		{
			pCmd->pExt->transferLen = JtTlv8103_getChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen , &tlvCount );
		}
		else
		{
			pCmd->pExt->transferLen = JtTlv8103_getDefChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen ) ;
		}
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
		gGPRSConTimeCnt = GET_TICKS();		// ��ʼ��ʱ
	}
	else if ( ev == UTP_REQ_FAILED  )	//��ȡʧ�ܣ����¶�ȡ
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
		// ����ǰ����ʱ�ı�
		pJt->blecfgParam.BleAdvInterval = Dt_convertToU32(&pJt->blecfgParam.BleAdvInterval,DT_UINT32);
	}
	else if( ev == UTP_TX_DONE )
	{
		// ���ͺ�,�ָ�
		pJt->blecfgParam.BleAdvInterval = Dt_convertToU32(&pJt->blecfgParam.BleAdvInterval,DT_UINT32);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
		// ����ǰ����ʱ�ı�
		pJt->bleEnCtrl = Dt_convertToU16(&pJt->bleEnCtrl,DT_UINT16);
	}
	else if( ev == UTP_TX_DONE )
	{
		// ���ͺ�,�ָ�
		pJt->bleEnCtrl = Dt_convertToU16(&pJt->bleEnCtrl,DT_UINT16);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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

		//���յ����ݺ�property����Ľṹ��ƥ�䣬��Ҫ���¸�ֵHwVer��FwVer

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
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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

	if (ev == UTP_REQ_SUCCESS)	//��ȡ�ɹ�
	{
		g_BleCfgParam_mirror.BleAdvInterval = Dt_convertToU32(&g_BleCfgParam_mirror.BleAdvInterval,DT_UINT32);
		//g_BleCfgParam_mirror
		//_JT808_DEBUGMSG("BLE\r\nName:%s",
		//	g_BleCfgParam_mirror.BleName );
		//_JT808_DEBUGMSG("Inter:%d,Power:%d\r\n",
		//	g_BleCfgParam_mirror.BleAdvInterval,g_BleCfgParam_mirror.BleAdvPower );
		//_JT808_DEBUGMSG("Adv:%s\r\n",g_BleCfgParam_mirror.BleAdvData);
		// �鿴�Ƿ���Ҫ����BLE Name
		//JtDevBleCfgParam
		if( 0 != memcmp( &g_BleCfgParam_mirror , &g_Jt.blecfgParam ,sizeof(JtDevBleCfgParam) ))
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
		}
		gSendCanCmdCnt = 0x00;
	}
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
	if (ev == UTP_REQ_SUCCESS)	//��ȡ�ɹ�
	{
		pJt->updatefileinfo.Updatefilelength = Dt_convertToU32(&pJt->updatefileinfo.Updatefilelength,DT_UINT32);

		PFL(DL_JT808,"Update file Len:%x,Ver:%d\r\n",
			g_Jt.updatefileinfo.Updatefilelength,g_Jt.updatefileinfo.UpdatefileVersion );
		
		gSendCanCmdCnt = 0 ;

		if( g_Jt.updatefileinfo.Updatefilelength < APPLICATION_SIZE  )
		{
			// ��ʼ���͸�������
			pJt->filecontentreq.fileOffset = 0 ;		
			pJt->filecontentreq.fileType = pJt->fileDesc.fileType ;
			Utp_DelaySendCmd( &g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT , 100 );		// ��һ�η���Ϊ 0
			PFL(DL_JT808,"Start Rev File Pack offset:%x,type:%d\r\n",
				pJt->filecontentreq.fileOffset , pJt->filecontentreq.fileType );
			// 
			EraseFirmwareAllArea();		// ɾ����������
		}
		else
		{
			PFL(DL_JT808,"Update File size >.\r\n ");
		}
	}
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
	if (ev == UTP_TX_START) 	// �ϱ�
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS) //��ȡ�ɹ�
	{
		gSendCanCmdCnt = 0 ;
		//����״̬�ɹ�

		// ����֮
		switch( pJt->setToOpState.OperationState )
		{
			case JT_STATE_SLEEP:
				pJt->opState = pJt->setToOpState.OperationState ;
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
				//gModuleSleepCnt = GET_TICKS();

				JT808CAN_Sleep();
				break;
			case JT_STATE_WAKEUP:
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
				//gModuleWakeupCnt = GET_TICKS();
				Fsm_SetActiveFlag(AF_JT808, True);
			
				break;
			default:break ;
		}
		PFL(DL_JT808,"����OpState:%d[%d]\r\n",pJt->setToOpState.OperationState,pJt->opState );
	}
	else if ( ev == UTP_REQ_FAILED )	//��ȡʧ�ܣ����¶�ȡ
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
		Utp_DelaySendCmd(&g_JtUtp, JTCMD_SET_OP_STATE, 500);		// ��Զ����״̬
	}
	return UTP_EVENT_RC_SUCCESS;
}

static void _SetOperationState(uint8_t Operation, uint8_t Parameter )
{
	g_Jt.setToOpState.OperationState = Operation ;
	g_Jt.setToOpState.StateParameter = Parameter ;
	Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
}

void CtrlBLE( uint16_t ctrl )
{
	g_Jt.bleEnCtrl = ctrl ;
	Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
	PFL(DL_JT808,"Ctrl BLE:%x\r\n",ctrl);
}


void JT808_CheckResetSerAddr(void)
{
	Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
}


// ��������

void SetComModeSleep(void)
{
	if( g_Jt.opState != JT_STATE_SLEEP )
	{
		//Utp_Reset(&g_JtUtp);		// �������ָ��?		_SetOperationState( JT_STATE_SLEEP , _OPERATION_SLEEP );
		_SetOperationState( JT_STATE_SLEEP , _OPERATION_SLEEP );
		PFL(DL_JT808,"JT808 Send Sleep Command\r\n");
	}
}

// ģ���Ѿ�����
Bool ComModeSleep(void)
{
	return ( g_Jt.opState == JT_STATE_SLEEP );
}
// ģ������׼������
Bool ComModeSleepIng(void)
{
	return ( ( g_Jt.opState != JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_SLEEP ) ) ;
}

// ģ���Ѿ�����
Bool ComModeWakeup(void)
{
	return ( g_Jt.opState == JT_STATE_WAKEUP );
}


// ģ�����ڻ���
Bool ComModeWakeupIng(void)
{
	return ( ( g_Jt.opState == JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_WAKEUP ) ) ;
}

// ����ģ�黽��
Bool ComIRQWakeup(void)
{
	extern FlagStatus modul_receive_flag;
	return modul_receive_flag == 1 ;
}

void SetComModeWakeup(void)
{
	if( g_Jt.opState == JT_STATE_SLEEP )
	{
		//Utp_Reset(&g_JtUtp);		// �������ָ��?		_SetOperationState( JT_STATE_WAKEUP , _OPERATION_WKUP );
		_SetOperationState( JT_STATE_WAKEUP , _OPERATION_WKUP );
		PFL(DL_JT808,"JT808 Send Wakup Command\r\n");
	}
}



void JT808_switchState(JT808* pJt, JT_state newState)
{
	if( newState > JT_STATE_OPERATION ) return ;
	
	if (pJt->opState == newState && newState != JT_STATE_INIT ) return;
	
	switch (newState)
	{
		case JT_STATE_INIT:
			// ��������
			// �Ѿ������������?
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				_SetOperationState( JT_STATE_PREOPERATION,_OPERATION_PRE);
			}
			// �������״�?			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			Fsm_SetActiveFlag(AF_JT808, True );
			break;
		case JT_STATE_SLEEP:
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;

			// �ر�CAN ������
			JT808CAN_Sleep();
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
			// ��������
			// �Ѿ������������?			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
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
		// ͨ��ģ��״̬�� ���ر���״̬��һ��ʱ
		if( pJt->opState != pJt->setToOpState.OperationState )
		{
			PFL(DL_JT808,"Change OPState[%d-%d]\r\n",pJt->opState ,pJt->setToOpState.OperationState );
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				pJt->opState = JT_STATE_INIT;		// ���¿�ʼ
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
	if( ev == UTP_CHANGED_BEFORE ) // �仯֮ǰ
	{
		uint16_t cnt_state = *(uint16_t*)pCmd->pExt->transferData ;
		PFL(DL_JT808,"Cur Cnt:%x\r\n", cnt_state );
		if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT ) // �Ѿ�����
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
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );// ��ȡ����
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
		// ��������
		//smsRevTime
		g_Jt.smsContext.smsText.smsRevTime = Dt_convertToU32( &g_Jt.smsContext.smsText.smsRevTime ,DT_UINT32 );
		// ��������
		PFL(DL_JT808,"SMS:%d,SMSCout:%d,Len:%d,Time:%d\r\n",
			g_Jt.smsContext.smsExist,g_Jt.smsContext.smsCount,
			g_Jt.smsContext.smsText.smsLen,g_Jt.smsContext.smsText.smsRevTime );
		// ���ж���
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
		Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
	  	JtTlv8900_proc(pCmd->pExt->transferData, pCmd->pExt->transferLen);
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

/*
	�Ƿ��������� -- �ռ�      and ���?> 60% and �Ƿ�����
*/
static Bool _CheckFirmwareUpdate(void)
{
	Adc* pAdc =  Adc_Get( ADC_18650_VOLTAGE );

	PFL(DL_JT808,"Check Firmware Updat,18650:%dmv\n",pAdc->newValue);

	return pAdc->newValue > 3900 ? true : false ;	// > 3.9v ���������
	
	// ֻҪ���� > 3.9v ����Կ�ʼ����
	//if( g_pdoInfo.isLowPow ) return false;
	
	//return true ;
}


UTP_EVENT_RC JT808_event_rcvFileData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	//PFL(DL_JT808,"rvcFileData :%d\r\n", ev );
	 if (ev == UTP_REQ_SUCCESS)
	 {
	 	switch( pJt->fileDesc.fileType )
	 	{
			case 0x01 :
				PFL(DL_JT808,"C8 Update File:%d\r\n",pJt->fileDesc.fileVerDesc);
				if( _CheckFirmwareUpdate() )
				{
					Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
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
		//pUtp->waitRspMs = pUtp->frameCfg->waitRspMsDefault;	//Ĭ�ϵȴ���Ӧ��ʱ��Ϊ1��
		//pUtp->maxTxCount = 3;		//Ĭ�ϵ��ط�����Ϊ3

		g_JtUtp.waitRspMs = 1000 * 10 ;	// 10s
	
		pCmd->pExt->transferLen = g_txlen;
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		g_txlen = 0;
		gSendCanCmdCnt = 0x00;
		//int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
		//JtTlv0900_updateMirror( g_txBuf , len );
		JtTlv0900_UpdateMirror();	// ����
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
	if( ev == UTP_TX_START )	// ���͵�����Ҫ��һ��
	{
		//����֮ǰ����ʱ--��ɴ��ģʽ�Ա㷢��
		pJt->filecontentreq.fileOffset = Dt_convertToU32( &pJt->filecontentreq.fileOffset , DT_UINT32 ); 		// ������Ҫȡ��

		//g_JtUtp.waitRspMs = 1000 ;	// 10s
	}
	else if( ev == UTP_TX_DONE )
	{
		//����֮�󣬻ظ�֮
		pJt->filecontentreq.fileOffset = Dt_convertToU32( &pJt->filecontentreq.fileOffset , DT_UINT32 );
	}
	else if( ev == UTP_GET_RSP )		// ��Ӧ������
	{
		//
	}
	else if(ev == UTP_REQ_SUCCESS )
	{
		pJt->filecontentrsq.fileOffset = Dt_convertToU32( &pJt->filecontentrsq.fileOffset , DT_UINT32);
		if( pJt->filecontentreq.fileOffset == pJt->filecontentrsq.fileOffset )
		{
			// дFlash
			WriteFirmware( pJt->filecontentrsq.fileOffset,pJt->filecontentrsq.fileData , 128 );
			
			pJt->filecontentreq.fileOffset += 128 ;			
			if( pJt->filecontentreq.fileOffset >= pJt->updatefileinfo.Updatefilelength )
			{
				if( CheckFirmware( pJt->updatefileinfo.Updatefilelength ) )
				{
					gJT808UpdateFirmware = true ;	// ��������
					//PFL(DL_JT808,"Check File OK ,Entern bootloader\r\n");
					//NVIC_SystemReset();
					PFL(DL_JT808,"Check File OK ,Wait bootloader...\r\n");
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
				//Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_FILE_CONTENT );		// ��������
				PFL(DL_JT808,"Rev File Next Pack :%d\r\n",pJt->filecontentreq.fileOffset );
			}
		}
		gSendCanCmdCnt = 0 ;

		Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);		// ���������㷢

		Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);	// ��ֹ������
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
			// ���?			EraseFirmwareInfoArea();
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

/*
	�յ�Beacon����,�յ�Beacon����֮���500ms��ʼ��������
	UUID
	Major
	Minor
	RSSI
	SOC
	Voltage

	LTV[02 01 05]


	ƻ����Beacon ��ʽ��0xFF
	1a ff 4c 00 02 15 fd a5 06 93 a4 e2 4f b1 af cf c6 eb 07 64 78 25 27 11 4c b9 c5
	:::
	4c 00 :ƻ����˾
	02 15  : TLV : 02��ʾBeacon 15��ʾ���ݳ���
	fd a5 06 93 a4 e2 4f b1 af cf c6 eb 07 64 78 25 // UUID
	27 11 											// Major
	4c b9 											// Minor
	c5												// twPower
*/

//4c 00 02 15 fd a5 06 93 a4 e2 4f b1 af cf c6 eb 07 64 78 25 27 11 4c b9 c5
static Bool _ConvertIbeacon_FF( int8_t rssi ,uint8_t *pAdv , BeaconCell *pBecaon )
{
	if( !pAdv || !pBecaon ) return False ;
	memcpy( pBecaon->miUUID , pAdv + 4 , 16 );
	pBecaon->miMajor = ( pAdv[21] << 8 ) | pAdv[22] ;
	pBecaon->miMinor = ( pAdv[23] << 8 ) | pAdv[24] ;
	pBecaon->miRSSI = rssi ;
	return True ;
}

//29 15 63 0C 0E F4 01 00 03 F0 2B E3 AD 34 92 27 11 14 E9
static Bool _ConvertIbeacon_16( int8_t rssi ,uint8_t *pAdv , BeaconCell *pBecaon )
{
	if( !pAdv || !pBecaon ) return False ;
	pBecaon->miSOC = pAdv[2];
	pBecaon->miVoltage = ( pAdv[3] << 8 ) | pAdv[4];
	return True ;
}
static Bool _ConvertIbeacon_09( int8_t rssi ,uint8_t *pAdv , BeaconCell *pBecaon )
{
	if( !pAdv || !pBecaon ) return False ;
	return True ;
}




static Bool _ConvertBleAdvToBeacon(BLEAdvContext *pAdvCntext, BeaconCell *pBecaon )
{
	uint8_t t = 0 , l = 0 , i = 0 ;
	if( !pAdvCntext || !pBecaon ) return False ;
	Bool result = False ;
	for( i = 0 ; i < 62 ; )
	{
		l = pAdvCntext->mADV[i++];
		if( l == 0 ) break ;
		t = pAdvCntext->mADV[i++];
		switch ( t )
		{
			case 0xFF :	// �鿴�Ƿ�Ϊƻ����Ibeacon
				if( l == 0x1A /*&& pAdvCntext->mADV[i+0] == 0x4C &&
					pAdvCntext->mADV[i+1] == 0x00 */&& pAdvCntext->mADV[i+2] == 0x02 && 
					pAdvCntext->mADV[i+3] == 0x15 )
				{
					_ConvertIbeacon_FF( pAdvCntext->miRSSI ,&pAdvCntext->mADV[i] , pBecaon );
					result = True ;
				}
				break;
			case 0x16 :	// ˽��Э��-- ����
				_ConvertIbeacon_16( pAdvCntext->miRSSI ,&pAdvCntext->mADV[i] , pBecaon );
				break ;
			case 0x09 :	// �㲥����
				_ConvertIbeacon_09( pAdvCntext->miRSSI ,&pAdvCntext->mADV[i] , pBecaon );
				break ;
			case 0x2B :	// �鿴�Ƿ�ΪMesh��Beacon
				break ;
			default :
				break ;
		}
		i += ( l - 1 ) ;
	}
	return result ;
}

static Bool _ChecBeaconUUID( BeaconCell *pBeacon )
{
	uint8 i = 0 ;
	if( !pBeacon ) return False ;

	for( i = 0 ; i < 16 ; i++ )
	{
		if( g_cfgInfo.BeaconUUID[i] != 0 ) break ;
	}
	if( i >= 16 ) return True ;

	for( i = 0 ; i < 16 ; i++ )
	{
		if( g_cfgInfo.BeaconUUID[i] != pBeacon->miUUID[i] ) break ;
	}

	return i >= 16 ? True : False ;
}

// ����и���
static void _UpdateBeacon( BeaconCell *pBeacon )
{
	uint8 i = 0 ;
	if( !pBeacon ) return ;

	// �ȸ���Ibeacon
	for( i = 0 ; i < gBleAdvCntextCnt ; i++ )
	{
		if( pBeacon->miMajor == gBeaconCntext[i].Major &&
			pBeacon->miMinor == gBeaconCntext[i].Minor )
		{
			break ;
		}
	}
	if( i < sizeof(gBeaconCntext)/sizeof(Beacon) )
	{
		gBeaconCntext[i].Major = pBeacon->miMajor;
		gBeaconCntext[i].Minor = pBeacon->miMinor;
		gBeaconCntext[i].RSSI = pBeacon->miRSSI;
		gBeaconCntext[i].soc = pBeacon->miSOC;
		gBeaconCntext[i].Voltage = pBeacon->miVoltage;
		if( i >= gBleAdvCntextCnt ) gBleAdvCntextCnt++ ;
	}
	PFL(DL_JT808,"Update Ibeacon[%d:%d]:%02X-%02X,%d,%d,%d\n",
		gBleAdvCntextCnt,i,
		pBeacon->miMajor,pBeacon->miMinor,pBeacon->miRSSI,
		pBeacon->miSOC,pBeacon->miVoltage );
}

UTP_EVENT_RC JT808_event_BeaconEvent(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	uint8 i = 0 ;
	BLEAdvContext *pAdvCntext = Null ;
	BeaconCell becaonCell = { 0x00 };
	if( ev == UTP_CHANGED_AFTER )
	{
		//PFL(DL_JT808,"Beacon Event:%d\n",pCmd->pExt->transferLen);
		for( i = 0 ; i < pCmd->pExt->transferLen/sizeof(BLEAdvContext) ; i += sizeof(BLEAdvContext) )
		{
			pAdvCntext = &pJt->bleBeaconCntext.BeaconADV[i];
			if( _ConvertBleAdvToBeacon( pAdvCntext , &becaonCell ) )
			{
				if( _ChecBeaconUUID( &becaonCell ) )
				{
					if( !gBeaconUpdate ) gBleAdvCntextCnt = 0 ;
					gBeaconUpdate = True ;
					gBeaconTick = GET_TICKS();
					_UpdateBeacon( &becaonCell );
					Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
				}
			}
		}
		memset( &pJt->bleBeaconCntext , 0 , sizeof(BeaconCntext));
	}
	else if( ev == UTP_GET_RSP )
	{
		
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_rcvBleData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{	
	// Ӧ�ô��� UTP_CHANGED_AFTER �¼�
	if( ev == UTP_CHANGED_AFTER )	// �յ�����
	{
		//
	}
	else if (ev == UTP_GET_RSP) 	// ��������
	{
		Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
		// ����������д�� pCmd->pExt->transferData		& pCmd->pExt->transferLen����
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
			Ble_Logout();	// ���֮
			PFL(DL_JT808,"BLE Connect[%02X:%02X:%02X:%02X:%02X:%02X]\r\n",
				g_Jt.bleState.bleConnectMAC[0],g_Jt.bleState.bleConnectMAC[1],
				g_Jt.bleState.bleConnectMAC[2],g_Jt.bleState.bleConnectMAC[3],
				g_Jt.bleState.bleConnectMAC[4],g_Jt.bleState.bleConnectMAC[5]);
		}
		else
		{
			PFL(DL_JT808,"Ble Disconnect\r\n");
			Ble_Logout();	// ���֮
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_bleAuthChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}



//���е�����д����¼�������������øûص�����?
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
		//��ȡ�����е��豸��Ϣ���л���OPERATION״̬
		const UtpCmd* pCmd = (UtpCmd*)param1;
		if (pCmd->cmd == JTCMD_CMD_GET_BLE_CFG/*JTCMD_CMD_GET_FILE_INFO*/)
		{
			_SetOperationState(JT_STATE_OPERATION,_OPERATION_OPE );
		}
	}
}
//gBeaconCntext ,gBleAdvCntextCnt
void GetBleBeaconInfo(  GetBeaconREQ *pBeaconREQ,BleBeaconPkt1*poutBle)
{
	for(uint8_t i = 0 ; i < gBleAdvCntextCnt ; i++ )
	{
		if( gBeaconCntext[i].Major == pBeaconREQ->major && 
			gBeaconCntext[i].Minor == pBeaconREQ->minor )
		{
			poutBle->rssi = gBeaconCntext[i].RSSI ;
			poutBle->soc = gBeaconCntext[i].soc ;			
			poutBle->vol = gBeaconCntext[i].Voltage;
		}
	}
}
extern void JTTlv0900_updateBeacon(Beacon *pBeacon , uint8 bleCnt);
void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_RUN)
	{
		// ����JtTlv9000����
		JtTlv0900_updateStorage();	//
		
		if (Utp_isIdle(&g_JtUtp) && (g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT) )	// ���� & ��������s
		{
			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
			if (len) 
			{
				g_txlen = len;		// ȫ������
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
				PFL(DL_JT808,"Send Bat Info:%d\r\n",len );
				// save 
				JtTlv0900_Cache( g_txBuf , len );
			}
			// Beacon����
			if( gBeaconUpdate )
			{
				if( GET_TICKS() - gBeaconTick > 500 )		// 500msû����,����Ϊ�������?
				{
					PFL(DL_JT808,"Send Beacon Info:%d[%d]\r\n",len ,gBleAdvCntextCnt);
					JTTlv0900_updateBeacon( gBeaconCntext ,gBleAdvCntextCnt  );
					// update Ble
					UpdateBleBeacon( gBeaconCntext ,gBleAdvCntextCnt );
					gBleAdvCntextCnt = 0x00;
					gBeaconUpdate = False ;
				}
				
			}
			// ���������ϱ�
			if( gLocationFlag )
			{
				gLocationFlag = False;
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_LOCATION_EXTRAS);
				PFL(DL_JT808,"Send Location Info\r\n" );
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
		case 15: printf("receive Utpcmd15!\r\n");rt_thread_mdelay(1); g_txlen = sprintf((char*)g_txBuf,"Hello Server"); ;Utp_SendCmd(&g_JtUtp, 0x15);break;
	}
 
}


MSH_CMD_EXPORT(JTcmd, JT808_sendCmd<uint8_t ind>);




#define	_CAN_TX_TIMEOUT_MS		(100)
extern can_trasnmit_message_struct transmit_message;
extern can_receive_message_struct  receive_message;
extern FlagStatus modul_receive_flag;
//xx �������ݵ����� һ����෢��?���ֽ� ����ʱ���ж��Ƿ����һ��?
int JT808_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmdΪCANЭ���PF
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


//xx �������Ͻ�������

void JT808_rxDataProc(const uint8_t* pData, int len)
{
	//Utp_RxData(&g_JtUtp, pData, len);
	CirBuffPush( &g_JtUtp.rxBuffCirBuff, pData ,len );
}



void JT808_timerProc()
{
	static uint16_t rev_timeout_cnt = 0 ;
	if( modul_receive_flag == SET )
	{
		modul_receive_flag = RESET;
		gCanbusRevTimeMS = GET_TICKS();
		
		g_pdoInfo.isCANOk = 1;
		rev_timeout_cnt = 0;
		gJT808Disconnect = False ;
	}
	else
	{
		if( GET_TICKS() - gCanbusRevTimeMS > _CAN_BUS_REV_TIMEOUT_MS )
		{
			PFL_WARNING("CAN Rev Timeout.reset can\r\n");
			can0_reset();
			gCanbusRevTimeMS = GET_TICKS();
			// �������״�?			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			g_pdoInfo.isCANOk = 0;
			if( ++rev_timeout_cnt > 12 )		// 1����δ�յ�CAN��������Ϊû������
			{
				if( !gJT808Disconnect )
				{
					PFL_WARNING("CAN Disconnect...\n");
				}
				gJT808Disconnect = True ;
				rev_timeout_cnt = 0x00 ;
			}
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
	Utp_Run(&g_JtUtp);
}


void JT808CAN_Sleep(void)
{
	can0_sleep();	// CAN ����
	Utp_Reset(&g_JtUtp);
	Fsm_SetActiveFlag(AF_JT808, False);		// ����״̬
}



#define		_SLEEP_TIMEOUT_MS			(15*1000)			// 15*1000 ģ������
#define		_WAKEUP_TIMEOUT_MS			(30*1000)			// 15*1000	ģ�黽�ѳ�ʱ


extern JT808ExtStatus gJT808ExtStatus ;

static void MouduleWork(void)
{
	if( ComModeSleep() )	// ģ���Ѿ�����---����Ƿ���Ҫ���Ѵ���
	{
		if( ComModeWakeupIng() )	// ���ڱ�����
		{
			if( GET_TICKS() - gModuleWakupIngTimeoutCnt > _WAKEUP_TIMEOUT_MS )
			{
				//��ʱҲû�취��ֻ������CAN
				can0_reset();
				gModuleWakupIngTimeoutCnt = GET_TICKS();		// ���¿�ʼ
				Utp_Reset(&g_JtUtp);
				SetComModeWakeup();
				PFL_WARNING("GPRS/GPS Wakeup Timeout,Sleep CAN\r\n");
			}
			JT808_Work();
		}
		else
		{
			// ����ģ���������� or ��MCU����
			if( !WorkMode_Sleep() && _JT808_EXT_SLEEP != gJT808ExtStatus )	// ��Χ����
			{
				gModuleWakupIngTimeoutCnt = GET_TICKS();	// ��ʱ
				can0_wakeup();
				Utp_Reset(&g_JtUtp);
				SetComModeWakeup();
				PFL(DL_JT808,"GPRS/GPS Wakeup[%d]...\r\n",gJT808ExtStatus );
				JT808_Work();
				if( gJT808ExtStatus == _JT808_EXT_WAKUP_IRQ )	// ����ģ����������
				{
					Pms_postMsg(PmsMsg_GPRSIrq, 0, 0);
				}
			}
		}
	}
	else					// ���δ����,�����Ƿ���Ҫ����
	{
		if( ComModeSleepIng() ) 	// ���ڽ�������
		{
			//���������Ƿ�ʱ
			if( GET_TICKS() - gModuleSleepIngTimeoutCnt > _SLEEP_TIMEOUT_MS )
			{
				//��ʱҲû�취��ֻ������CAN
				can0_reset();
				Utp_Reset(&g_JtUtp);
				SetComModeSleep();	// ׼����������
				gModuleSleepIngTimeoutCnt = GET_TICKS();
				PFL_WARNING("GPRS/GPS Sleep Timeout,CAN Sleep\r\n");
			}
		}
		else					// û�����߼���Ƿ���Ҫ������
		{
			if( _JT808_EXT_SLEEP == gJT808ExtStatus )
			{
				SetComModeSleep();	// ׼����������
				gModuleSleepIngTimeoutCnt = GET_TICKS();		// ��ʱ
			}
			else if(_JT808_EXT_BRIEF_WAKUP == gJT808ExtStatus ) // ��ʱ���� ��λ30s��,��������5���Ӻ������
			{
				// ��ʱ15s������
				if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT &&
					g_Jt.devState.cnt & _GPS_FIXE_BIT &&
				GET_TICKS() - gModuleWakupIngTimeoutCnt > 3*60*1000  )
				{
					SetComModeSleep();	// ׼����������
					gModuleSleepIngTimeoutCnt = GET_TICKS();		// ��ʱ
					PFL(DL_JT808,"JT808 BRIEF Wakeup GPRS&GPS fix,Sleep\n");
					gJT808ExtStatus = _JT808_EXT_SLEEP; // ���֮
				}
				else if( GET_TICKS() - gModuleWakupIngTimeoutCnt > 5*60*1000 )
				{
					SetComModeSleep();	// ׼����������
					gModuleSleepIngTimeoutCnt = GET_TICKS();		// ��ʱ
					PFL(DL_JT808,"JT808 BRIEF Wakeup timeout > 5min,Sleep\n");
					gJT808ExtStatus = _JT808_EXT_SLEEP; // ���֮
				}
			}
		}
		JT808_Work();
	}
}



/*
	JT808����ʱ---��MCU��������( WorkMode_Sleep() ),Ҳ���ܲ�����( !WorkMode_Sleep())
	JT808���ܱ���������ǻ��ѣ���Χģ����������,��MCU����(MCUδ����ʱ����,MCU����ʱ����)

	// ����һ�ֿ���,ģ��û�н� or ģ�黵��
*/
void JT808_run(void)
{
	static Bool lastJT808Disconnect = False; 
	
	if( lastJT808Disconnect != gJT808Disconnect )
	{
		if( !lastJT808Disconnect )	// �Ͽ�
		{
			// can������
			Utp_Reset(&g_JtUtp);
			can0_sleep();
			Fsm_SetActiveFlag(AF_JT808, False);
			Pms_postMsg(pmsMsg_GPRSPlugOut, 0, 0);
			PFL(DL_JT808,"JT808 CAN Plug out\n");
		}
		else						// ����
		{
			// can ��ʼ��
			JT808_start();
			Pms_postMsg(PmsMsg_GPRSPlugIn, 0, 0);
			PFL(DL_JT808,"JT808 CAN Plug In\n");
		}
		lastJT808Disconnect = gJT808Disconnect ;
	}
	
	if( lastJT808Disconnect )		// CAN δ����
	{
		// do nothing
		Fsm_SetActiveFlag(AF_JT808, False );
	}
	else						// CAN �Ѿ�����
	{
		MouduleWork();
	}
}

#endif //

Bool JT808_wakeup(void)
{
	return True;
}

Bool JT808_Sleep(void)
{
	// ����֮
	if( !gJT808Disconnect )
	{
		SetComModeSleep();	// ׼����������
		gModuleSleepIngTimeoutCnt = GET_TICKS();		// ��ʱ
	}
	return True;
}

void JT808_start(void)
{
	if( !gJT808Disconnect )
	{
		can0_wakeup();
		Utp_Reset(&g_JtUtp);		// ������з������� //����Ӳ����ʹ���ж�
		g_Jt.opState = JT_STATE_UNKNOWN ;
		JT808_switchState(g_pJt, JT_STATE_INIT);

		gCanbusRevTimeMS = GET_TICKS();	
	}
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
	// �������
	// Name
	{
		uint8_t j = i , len = 0 ;		
		_adv[i++] = 0;// ���� 1 + sizeof(_BLE_NAME) - 1;
		_adv[i++] = 0x09;	// name
		len = sprintf( (char*)_adv + i ,_BLE_NAME , _DEV_MODEL, mac[3],mac[4],mac[5]);
		_adv[j] = 1 + len ;
		i += len ;
	}
	//
	memcpy( _adv + i , _adv_type , sizeof(_adv_type) );
	i += sizeof(_adv_type);
	// MAC ��ַ
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
*���±�����Э�����������Э��ʹ��?************************************************/
	
	#define JT_CMD_SIZE 24
	static uint8_t g_protocolVer = 1;	//����Э��汾��?	//static uint8_t g_updatefiletype = 1; // �п�
	static uint8_t g_rxBuf[192];	
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//λ��Խ��ǰ���������ȼ�Խ��
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
		{&g_JtCmdEx[15],UTP_WRITE, JTCMD_CMD_SET_LOCATION_EXTRAS, "SetLocationExtras", (uint8_t*)&g_Jt.locationExtras, sizeof(LocationExtras), Null, 0, (UtpEventFn)JT808_event_setLocationExtras},
		// EVENT 
		// ���� & GPS
		{&g_JtCmdEx[16],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},
		{&g_JtCmdEx[17],UTP_EVENT, JTCMD_EVENT_DEV_STATE_LOCATION, "LocationChanged", (uint8_t*)& g_Jt.locatData, sizeof(Jt_LocationData), Null, 0, (UtpEventFn)JT808_event_LocationChanged},
		{&g_JtCmdEx[18],UTP_EVENT, JTCMD_EVT_RCV_SVR_DATA, "RcvSvrData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvSvrData},
		{&g_JtCmdEx[19],UTP_EVENT, JTCMD_EVT_RCV_FILE_DATA, "RcvFileData", (uint8_t*)&g_Jt.fileDesc, sizeof(FileVersionDesc), Null, 0, (UtpEventFn)JT808_event_rcvFileData},
		// ����
		{&g_JtCmdEx[20],UTP_EVENT, JTCMD_BLE_EVT_AUTH, "BleAuth", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null , 0 , (UtpEventFn)JT808_event_bleAuthChanged },
		{&g_JtCmdEx[21],UTP_EVENT, JTCMD_BLE_EVT_CNT, "BleCnt", (uint8_t*)&g_Jt.bleState, sizeof(Jt_BleState) , Null , 0 , (UtpEventFn)JT808_event_bleStateChanged },
		//
		
		{&g_JtCmdEx[22],UTP_EVENT, JTCMD_BLE_EVT_BEACON, "BleBeacon", (uint8_t*)&g_Jt.bleBeaconCntext, sizeof(BeaconCntext), Null, 0, (UtpEventFn)JT808_event_BeaconEvent},
		{&g_JtCmdEx[23],UTP_EVENT, JTCMD_BLE_RCV_DAT, "BleRcvDat", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), (uint8_t*)g_txBuf, 0/*sizeof(g_txBuf)*/, (UtpEventFn)JT808_event_rcvBleData},
	}; 
	
	static const UtpCfg g_cfg =
	{
		.cmdCount = JT_CMD_SIZE,
		.cmdArray = g_JtCmd,
		.TxFn = JT808_txData,
		.TresferEvent = (UtpEventFn)JT808_utpEventCb,
		.pCbObj = &g_Jt,
	};

	static const Obj obj = { "JT808", JT808_start, (ObjFn)JT808_Sleep, JT808_run };
	ObjList_add(&obj);
	g_Jt.opState = JT_STATE_UNKNOWN;	//��ʼ��Ϊһ��UNKNOWNֵ

	{
		_UpdataBleAdvData( (uint8_t*)g_Jt.bleproperty.BleMac );
		g_Jt.blecfgParam.BleAdvInterval = 30;	
		g_Jt.bleEnCtrl = 0x01 ;
	}
	// �ն˳�ʼ��
	{
		g_Jt.property.protocolVer = 0x00;
		g_Jt.property.devClass = _DEV_TYPE;
		strcpy( (char*)g_Jt.property.devModel,_DEV_MODEL);
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
//	static uint8_t last_module_st = 0 ;	// Ĭ��ģ�鲻����
//	uint8_t cur_module_st = 0 ;
//	while (1)
//	{
		// �ȴ�״̬
//		JT808_run();
//		rt_thread_mdelay(10);
//	}
//}

//void Jt808TaskInit(void)
//{
//	rt_thread_t jt808_task_tid = rt_thread_create("JT808",/* �߳����� */
//		Jt808Task_thread_entry, NULL,
//		512, 3, 10); //
//	JT808_start();
//	rt_thread_startup(jt808_task_tid);
//}

#endif //#ifdef CANBUS_MODE_JT808_ENABLE


