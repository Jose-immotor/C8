
#include "Common.h"
#include "Pms.h"
#include "Battery.h"
#include "Modbus.h"

static Battery g_Bat[2];
static Pms g_pms;
Mod* g_pModBus = &g_pms.modBus;

static Battery* g_pActiveBat;	//当前正砸通信的电池
static BmsRegCtrl g_regCtrl;		//电池控制

//以下是命令参数定义
const static uint8_t g_bmsID[]    = { 0x00, 0x00, 0x00, 34 };
const static uint8_t g_bmsInfo1[] = { (uint8)(BMS_REG_INFO_ADDR_1 >> 8), (uint8)BMS_REG_INFO_ADDR_1, (uint8)(BMS_REG_INFO_COUNT_1 >> 8), (uint8)BMS_REG_INFO_COUNT_1};
const static uint8_t g_bmsInfo2[] = { (uint8)(BMS_REG_INFO_ADDR_2 >> 8), (uint8)BMS_REG_INFO_ADDR_2, (uint8)(BMS_REG_INFO_COUNT_2 >> 8), (uint8)BMS_REG_INFO_COUNT_2};
const static uint8_t g_bmsCtrl[]  =  { (uint8)(BMS_REG_CTRL_ADDR >> 8) , (uint8)BMS_REG_CTRL_ADDR  , (uint8)(BMS_REG_CTRL_COUNT >> 8)  , (uint8)BMS_REG_CTRL_COUNT };
const static uint8_t g_bmsWCtrl[] = { 0x02, 0x00};

static int Pms_Tx(const uint8_t* pData, int len);

#define BMS_CMD_COUNT 5
static ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
/*电池槽位0的命令配置表*******************************************************************/
const static ModCmd g_Bms0Cmds[BMS_CMD_COUNT] =
{
	{&g_BmsCmdEx[0], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0ID"   , &g_Bat[0].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID, 4},
	{&g_BmsCmdEx[1], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info1", &g_Bat[0].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1, 4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[2], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info2", &g_Bat[0].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2, 4},
	{&g_BmsCmdEx[3], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Ctrl" , &g_Bat[0].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl, 4},

	{&g_BmsCmdEx[4], BMS_WRITE_CTRL, MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg0-Ctrl", &g_Bat[0].bmsCtrl.ctrl, 2, (void*)g_bmsWCtrl, 2,  &g_Bat[0].bmsCtrl_mirror.ctrl},
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
	{&g_BmsCmdEx[0], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1ID"   , &g_Bat[1].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID, 4},
	{&g_BmsCmdEx[1], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info1", &g_Bat[1].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1, 4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[2], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info2", &g_Bat[1].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2, 4},
	{&g_BmsCmdEx[3], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Ctrl" , &g_Bat[1].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl, 4},
																							 
	{&g_BmsCmdEx[4], BMS_WRITE_CTRL, MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg1-Ctrl", &g_Bat[1].bmsCtrl.ctrl, 2, (void*)g_bmsWCtrl, 2,  &g_Bat[1].bmsCtrl_mirror.ctrl},
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

/********************************************************************/
void Pms_setPreDischg(Bool en)	{ g_regCtrl.preDischg   = en; Bat_setPreDischg(g_pActiveBat, en); }
void Pms_setDischg(Bool en)		{ g_regCtrl.dischgEn    = en; Bat_setDischg(g_pActiveBat, en); }
void Pms_setChg(Bool en)		{ g_regCtrl.chgEn       = en; Bat_setChg(g_pActiveBat, en); }
void Pms_setSleep(Bool en)		{ g_regCtrl.sleepEn     = en; Bat_setSleep(g_pActiveBat, en); }
void Pms_setDeepSleep(Bool en)	{ g_regCtrl.deepSleepEn = en; Bat_setDeepSleep(g_pActiveBat, en); }

static int Pms_Tx(const uint8_t* pData, int len)
{
	//调用NFC发送数据命令
	return len;
}

//从总线接收到的数据，调用该函数处理
void Pms_Rx(int nfcPort, const uint8_t* pData, int len)
{
	//从NFC驱动接收数据
	Mod_RxData(g_pModBus, pData, len);
}

void Pms_postMsg(uint8_t msgId, uint32_t param1, uint32_t param2)
{

}

void Pms_plugIn(int port)
{
	Bat_msg(&g_Bat[port], BmsMsg_batPlugIn, 0, 0);
}

void Pms_plugOut(int port)
{
	Battery* pBat = (port == 0) ? &g_Bat[1] : &g_Bat[0];

	Bat_msg(&g_Bat[port], BmsMsg_batPlugout, 0, 0);

	if(g_Bat[0].presentStatus == BAT_NOT_IN && g_Bat[1].presentStatus == BAT_NOT_IN)
	{
		*((uint16*)& g_regCtrl) = 0;
	}
}

void Pms_SwitchPort()
{
	Battery* pBat = (g_pActiveBat->port == 0) ? &g_Bat[1] : &g_Bat[0];

	if (!Mod_isIdle(g_pModBus)) return;

	if (!Bat_isReadyFroInquery(g_pActiveBat))
	{
		if (Bat_isReadyFroInquery(pBat))
		{
			Bat_msg(g_pActiveBat, BmsMsg_deactive, 0, 0);
			Mod_SwitchCfg(g_pModBus, (g_pActiveBat->port == 0) ? &g_Bat1Cfg : &g_Bat0Cfg);
			Bat_msg(pBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
			g_pActiveBat = pBat;
		}
	}
}

static void Pms_switchStattus(PmsOpStatus newStatus)
{
	if (newStatus == g_pms.opStatus) return;

	if (newStatus == PMS_ACC_OFF)
	{

	}
	else if (newStatus == PMS_ACC_ON)
	{

	}
	else if (newStatus == PMS_SLEEP)
	{

	}
	else if (newStatus == PMS_DEEP_SLEEP)
	{

	}
}

static void Pms_fsm_accOff(uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == PmsMsg_accOff)
	{
		Pms_switchStattus(PMS_ACC_OFF);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{

	}
	else if (msgId == PmsMsg_batPlugout)
	{

	}
}

static void Pms_fsm_accOn(uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == PmsMsg_accOff)
	{
		Pms_switchStattus(PMS_ACC_OFF);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{

	}
	else if (msgId == PmsMsg_batPlugout)
	{

	}
}

static void Pms_fsm_sleep(uint8_t msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_deepSleep(uint8_t msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_anyStatusDo(uint8_t msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm(uint8_t msgId, uint32_t param1, uint32_t param2)
{
	struct
	{
		BmsOpStatus opStatus;
		void (*OpFun)(uint8_t msgId, uint32_t param1, uint32_t param2);
	}
	const static g_fsms[] =
	{
		{PMS_ACC_OFF, Pms_fsm_accOff},
		{PMS_ACC_ON	, Pms_fsm_accOn},
		{PMS_SLEEP	, Pms_fsm_sleep},
		{PMS_DEEP_SLEEP, Pms_fsm_deepSleep},
	};

	Pms_fsm_anyStatusDo(msgId, param1, param2);

	for (int i = 0; i < GET_ELEMENT_COUNT(g_fsms); i++)
	{
		if (g_fsms[i].opStatus == g_pms.opStatus)
		{
			g_fsms[i].OpFun(msgId, param1, param2);
			break;
		}
	}
}

void Pms_run()
{
	Bat_run(&g_Bat[0]);
	Bat_run(&g_Bat[1]);
	Pms_SwitchPort();
}

void Pms_start()
{
	//启动NFC驱动

}

void Pms_init()
{
	static const Obj obj = { "Pms", Pms_start, Null, Pms_run };
	ObjList_Add(&obj);

	Mod_Init(g_pModBus, &g_Bat0Cfg, &g_frameCfg);

	Bat_init(&g_Bat[0], 0, &g_Bat0Cfg);
	Bat_init(&g_Bat[1], 1, &g_Bat1Cfg);

	g_pActiveBat = &g_Bat[0];
	Queue_init(&g_pms.msgQueue, g_pms.msgBuf, sizeof(Message), GET_ELEMENT_COUNT(g_pms.msgBuf));
}
