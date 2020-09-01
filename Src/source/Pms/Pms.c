
#include "Common.h"
#include "Pms.h"
#include "Battery.h"
#include "Modbus.h"
#include "JT808.h"

Battery g_Bat[MAX_BAT_COUNT];
static Pms g_pms;
Mod* g_pModBus = &g_pms.modBus;

static Battery* g_pActiveBat;	//当前正砸通信的电池
static BmsRegCtrl g_regCtrl;		//电池控制

//以下是命令参数定义
const static uint8_t g_bmsID_readParam[]    = { 0x00, 0x00, 0x00, 33 };
const static uint8_t g_bmsInfo1_readParam[] = { (uint8)(BMS_REG_INFO_ADDR_1 >> 8), (uint8)BMS_REG_INFO_ADDR_1, (uint8)(BMS_REG_INFO_COUNT_1 >> 8), (uint8)BMS_REG_INFO_COUNT_1};
const static uint8_t g_bmsInfo2_readParam[] = { (uint8)(BMS_REG_INFO_ADDR_2 >> 8), (uint8)BMS_REG_INFO_ADDR_2, (uint8)(BMS_REG_INFO_COUNT_2 >> 8), (uint8)BMS_REG_INFO_COUNT_2};
const static uint8_t g_bmsCtrl_readParam[]  =  { (uint8)(BMS_REG_CTRL_ADDR >> 8) , (uint8)BMS_REG_CTRL_ADDR  , (uint8)(BMS_REG_CTRL_COUNT >> 8)  , (uint8)BMS_REG_CTRL_COUNT };
const static uint8_t g_bmsCtrl_writeParam[] = { 0x02, 0x00};

static int Pms_Tx(const uint8_t* pData, int len);

#define BMS_CMD_COUNT 5
static ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
/*电池槽位0的命令配置表*******************************************************************/
const static ModCmd g_Bms0Cmds[BMS_CMD_COUNT] =
{
	{&g_BmsCmdEx[0], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0ID"   , &g_Bat[0].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID_readParam, 4},
	{&g_BmsCmdEx[1], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info1", &g_Bat[0].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1_readParam, 4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[2], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info2", &g_Bat[0].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2_readParam, 4},
	{&g_BmsCmdEx[3], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Ctrl" , &g_Bat[0].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl_readParam, 4},

	{&g_BmsCmdEx[4], BMS_WRITE_CTRL, MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg0-Ctrl", &g_Bat[0].bmsCtrl.ctrl, 2, (void*)g_bmsCtrl_writeParam, 2,  &g_Bat[0].bmsCtrl_mirror.ctrl},
};

const static ModCfg g_Bat0Cfg =
{
	.port = 0,
	.cmdCount = BMS_CMD_COUNT,
	.cmdArray = g_Bms0Cmds,
	.pCbObj = &g_Bat[0],
	.TresferEvent = (ModEventFn)Bat_event,
	.TxFn = Pms_Tx,
};

/*电池槽位1的命令配置表*******************************************************************/
const static ModCmd g_Bms1Cmds[BMS_CMD_COUNT] =
{
	{&g_BmsCmdEx[0], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1ID"   , &g_Bat[1].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID_readParam, 	4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[1], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info1", &g_Bat[1].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1_readParam,4},
	{&g_BmsCmdEx[2], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info2", &g_Bat[1].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2_readParam,4},
	{&g_BmsCmdEx[3], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Ctrl" , &g_Bat[1].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl_readParam, 4},
																							 
	{&g_BmsCmdEx[4], BMS_WRITE_CTRL, MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg1-Ctrl", &g_Bat[1].bmsCtrl.ctrl, 2, (void*)g_bmsCtrl_writeParam, 2,  &g_Bat[1].bmsCtrl_mirror.ctrl},
};
const static ModCfg g_Bat1Cfg =
{
	.port = 0,
	.cmdCount = BMS_CMD_COUNT,
	.cmdArray = g_Bms1Cmds,
	.pCbObj = &g_Bat[1],
	.TresferEvent = (ModEventFn)Bat_event,
	.TxFn = Pms_Tx,
};

/*Modbus传输协议帧配置*****************************************************/
static uint8_t g_modBusTxBuf[128];
static uint8_t g_modBusRxBuf[128];
const static ModFrameCfg g_frameCfg =
{
	.txBuf = g_modBusTxBuf,
	.txBufLen = sizeof(g_modBusTxBuf),
	.rxBuf = g_modBusRxBuf,
	.rxBufLen = sizeof(g_modBusRxBuf),

	.waitRspMsDefault = 1000,
	.rxIntervalMs = 1,
	.sendCmdIntervalMs = 100,
};

//打印电池信息
void BatteryDescDump(const Battery* desc)
{
	const uint8_t* pByte = (uint8_t*)&(desc->bmsID.sn34);
	Printf("\tPort[%d][%02X%02X%02X%02X%02X%02X]:\n"
		,desc->port, pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5]);
	Printf("\t\tsoc(0.1%)=%d\r\n",bigendian16_get((uint8*)(&desc->bmsInfo.soc)));
	Printf("\t\tvolt(0.01V)=%d\r\n",bigendian16_get((uint8*)(&desc->bmsInfo.tvolt)));
	Printf("\t\tcurr(0.01A)=%d\r\n",bigendian16_get((uint8*)(&desc->bmsInfo.tcurr)));
	Printf("\t\tstate=0x%04X\r\n",bigendian16_get((uint8*)(&desc->bmsInfo.state)));
}

void BatteryDump(void)
{
	int i = 0;
	Battery* pPkt = Null;

	Printf("Battery info:\n");
//	BAT_DUMP(batteryCount);
	for(i = 0; i < 2; i++)
	{
		pPkt = &g_Bat[i];
		if(pPkt->presentStatus)
		{
			BatteryDescDump(pPkt);
		}
	}
}

/********************************************************************/

static Pms_FsmFn Pms_findStatusProcFun(PmsOpStatus status);

void Pms_setPreDischg(Bool en)	{ g_regCtrl.preDischg   = en; Bat_setPreDischg(g_pActiveBat, en); }
void Pms_setDischg(Bool en)		{ g_regCtrl.dischgEn    = en; Bat_setDischg(g_pActiveBat, en); }
void Pms_setChg(Bool en)		{ g_regCtrl.chgEn       = en; Bat_setChg(g_pActiveBat, en); }
void Pms_setSleep(Bool en)		{ g_regCtrl.sleepEn     = en; Bat_setSleep(g_pActiveBat, en); }
void Pms_setDeepSleep(Bool en)	{ g_regCtrl.deepSleepEn = en; Bat_setDeepSleep(g_pActiveBat, en); }

static int Pms_Tx(const uint8_t* pData, int len)
{
	//调用NFC发送数据命令
	NfcCardReader_Send(&g_pms.cardReader, g_pActiveBat->port, pData, len);
	return len;
}

//从总线接收到的数据，调用该函数处理
void Pms_Rx(int nfcPort, const uint8_t* pData, int len)
{
	//从NFC驱动接收数据
	if (g_pActiveBat->port != nfcPort) return;

	Mod_RxData(g_pModBus, pData, len);
}

void Pms_postMsg(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

void Pms_plugIn(uint8_t port)
{
	Bat_msg(&g_Bat[port], BmsMsg_batPlugIn, 0, 0);
}

void Pms_plugOut(uint8_t port)
{
	//Battery* pBat = (port == 0) ? &g_Bat[1] : &g_Bat[0];

	Bat_msg(&g_Bat[port], BmsMsg_batPlugout, 0, 0);

	if(g_Bat[0].presentStatus != BAT_NOT_IN && g_Bat[1].presentStatus != BAT_NOT_IN)
	{
		*((uint16*)& g_regCtrl) = 0;
	}
}

void Pms_SwitchPort()
{
	Battery* pBat = (g_pActiveBat->port == 0) ? &g_Bat[1] : &g_Bat[0];

	if (!Mod_isIdle(g_pModBus)) return;

	Bat_msg(g_pActiveBat, BmsMsg_deactive, 0, 0);
	Mod_SwitchCfg(g_pModBus, (g_pActiveBat->port == 0) ? &g_Bat1Cfg : &g_Bat0Cfg);
	Bat_msg(pBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
	g_pActiveBat = pBat;
}

static void Pms_switchStatus(PmsOpStatus newStatus)
{
	if (newStatus == g_pms.opStatus) return;

	g_pms.statusSwitchTicks = GET_TICKS();

	if (newStatus == PMS_ACC_OFF)
	{
		Pms_setDischg(True);
	}
	else if (newStatus == PMS_ACC_ON)
	{
		Pms_setDischg(True);
	}
	else if (newStatus == PMS_SLEEP)
	{
		Pms_setDischg(True);
	}
	else if (newStatus == PMS_DEEP_SLEEP)
	{
		Pms_setDischg(False);
	}

	g_pms.opStatus = newStatus;
	g_pms.Fsm = Pms_findStatusProcFun(newStatus);
}

static void Pms_fsm_accOff(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == PmsMsg_run)
	{
		if (g_pJt->isLocation)
		{
			Pms_switchStatus(PMS_SLEEP);
		}
	}
	else if (msgId == PmsMsg_accOn)
	{
		Pms_switchStatus(PMS_ACC_ON);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{
		Pms_plugIn((uint8_t)param1);
	}
	else if (msgId == PmsMsg_batPlugOut)
	{
		Pms_plugOut((uint8_t)param1);
	}
}

static void Pms_fsm_accOn(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == PmsMsg_accOff)
	{
		Pms_switchStatus(PMS_ACC_OFF);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{
		Pms_plugIn((uint8_t)param1);
	}
	else if (msgId == PmsMsg_batPlugOut)
	{
		Pms_plugOut((uint8_t)param1);
	}
}

static void Pms_fsm_sleep(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_CANErr(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_deepSleep(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

//在任何状态下都要处理的消息函数
static void Pms_fsm_anyStatusDo(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

//查找状态响应的处理函数
static Pms_FsmFn Pms_findStatusProcFun(PmsOpStatus status)
{
	struct
	{
		PmsOpStatus opStatus;
		Pms_FsmFn OpFun;
	}
	const static g_fsms[] =
	{
		{PMS_ACC_OFF	, Pms_fsm_accOff},
		{PMS_ACC_ON		, Pms_fsm_accOn},
		{PMS_SLEEP		, Pms_fsm_sleep},
		{PMS_DEEP_SLEEP , Pms_fsm_deepSleep},
		{PMS_CAN_ERR	, Pms_fsm_CANErr},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_fsms); i++)
	{
		if (g_fsms[i].opStatus == g_pms.opStatus) return  g_fsms[i].OpFun;
	}

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

static void Pms_fsm(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	Pms_SwitchPort();
	Pms_fsm_anyStatusDo(msgId, param1, param2);
	g_pms.Fsm(msgId, param1, param2);
}

NfcCardReaderEventRc Pms_cardReaderEventCb(Pms* pms, NfcCardReaderEvent ev)
{
	if (ev == CARD_EVENT_SEARCH_SUCCESS)
	{
		Pms_postMsg(PmsMsg_batPlugIn, g_pms.cardReader.port, 0);
	}
	else if (ev == CARD_EVENT_SEARCH_FAILED)
	{
		Pms_postMsg(PmsMsg_batPlugOut, g_pms.cardReader.port, 0);
	}
	else if (ev == CARD_EVENT_RX_DATA_SUCCESS)
	{
		Pms_Rx(g_pms.cardReader.port, g_pms.cardReader.rxBuf, g_pms.cardReader.rxLen);
	}
	else if (ev == CARD_EVENT_RX_DATA_FAILED)
	{
		Mod_busErr(g_pModBus, BUS_ERR_RX_FAILED);
	}

	return CARD_EVENT_RC_SUCCESS;
}

void Pms_run()
{
//	NfcCardReader_run(&g_pms.cardReader);
	Mod_Run(g_pModBus);
	Bat_run(&g_Bat[0]);
	Bat_run(&g_Bat[1]);

	Pms_fsm(PmsMsg_run, 0, 0);
}

void Pms_start()
{
	//启动NFC驱动
	NfcCardReader_start(&g_pms.cardReader);
	Pms_switchStatus(PMS_ACC_OFF);
	//查询电池设备信息
	Bat_msg(g_pActiveBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
}

void Pms_init()
{
	static const Obj obj = { "Pms", Pms_start, Null, Pms_run };
	ObjList_add(&obj);

	Mod_Init(g_pModBus, &g_Bat0Cfg, &g_frameCfg);

	Bat_init(&g_Bat[0], 0, &g_Bat0Cfg);
	Bat_init(&g_Bat[1], 1, &g_Bat1Cfg);

	g_pActiveBat = &g_Bat[0];
	Queue_init(&g_pms.msgQueue, g_pms.msgBuf, sizeof(Message), GET_ELEMENT_COUNT(g_pms.msgBuf));

	NfcCardReader_init(&g_pms.cardReader, (NfcCardReader_EventFn)Pms_cardReaderEventCb, &g_pms);
}
