
/*

电池在位置位：读取电池只读数据(BMS_READ_INFO_1)，接收数据完成；
电池在位清除：读取电池数据失败（以后再完善增加失败次数）

*/

#include "Common.h"
#include "Pms.h"
#include "Battery.h"
#include "Modbus.h"
#include "JT808.h"
#include "fm175Drv.h"
#include "workmode.h"
#include "Bat.h"

//#define		PMS_DEBUG_MSG(fmt,...)		Printf(fmt,##__VA_ARGS__)

#define		PMS_DEBUG_MSG(fmt,...)



Battery g_Bat[MAX_BAT_COUNT];
static Pms g_pms;
Mod* g_pModBus = &g_pms.modBus;

static Battery* g_pActiveBat;	//当前正砸通信的电池
static BmsRegCtrl g_regCtrl;		//电池控制

DrvIo* g_pLockEnIO = Null;

//以下是命令参数定义
const static uint8_t g_bmsID_readParam[]    = { 0x00, 0x00, (uint8)((sizeof(BmsReg_info)/2) >> 8 ), (uint8)(sizeof(BmsReg_info)/2)/*0x00, 33*/ };
const static uint8_t g_bmsInfo1_readParam[] = { (uint8)(BMS_REG_INFO_ADDR_1 >> 8), (uint8)BMS_REG_INFO_ADDR_1, (uint8)(BMS_REG_INFO_COUNT_1 >> 8), (uint8)BMS_REG_INFO_COUNT_1};
const static uint8_t g_bmsInfo2_readParam[] = { (uint8)((BMS_REG_INFO_ADDR_2+1) >> 8), (uint8)(BMS_REG_INFO_ADDR_2+1), (uint8)(BMS_REG_INFO_COUNT_2 >> 8), (uint8)BMS_REG_INFO_COUNT_2};
const static uint8_t g_bmsCtrl_readParam[]  =  { (uint8)(BMS_REG_CTRL_ADDR >> 8) , (uint8)BMS_REG_CTRL_ADDR  , (uint8)(BMS_REG_CTRL_COUNT >> 8)  , (uint8)BMS_REG_CTRL_COUNT };
const static uint8_t g_bmsCtrl_writeParam[] = { 0x02, 0x00};

static int Pms_Tx(const uint8_t* pData, int len);

#define BMS_CMD_COUNT 5
static ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
//ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
/*电池槽位0的命令配置表*******************************************************************/
//const static 
ModCmd g_Bms0Cmds[BMS_CMD_COUNT] =
{
	{&g_BmsCmdEx[0], BMS_WRITE_CTRL ,MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg0-Ctrl", &g_Bat[0].writeBmsCtrl , 2				  , (void*)g_bmsCtrl_writeParam, 2,  &g_Bat[0].bmsCtrl},//bmsInfo.state},//&g_Bat[0].writeBmsCtrl_mirror},
	{&g_BmsCmdEx[1], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0ID"   , &g_Bat[0].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID_readParam, 4, Null,(ModEventFn)Bat_event_readBmsID},
	{&g_BmsCmdEx[2], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info1", &g_Bat[0].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1_readParam, 4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[3], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Info2", &g_Bat[0].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2_readParam, 4,Null,(ModEventFn)Bat_event_readBmsInfo2},
	{&g_BmsCmdEx[4], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms0Ctrl" , &g_Bat[0].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl_readParam, 4},
	
};

//const 
static ModCfg g_Bat0Cfg =
{
	.port = 0,
	.antselct = 1,
	.cmdCount = BMS_CMD_COUNT,
	.cmdArray = g_Bms0Cmds,
	.pCbObj = &g_Bat[0],
	.TresferEvent = (ModEventFn)Bat_event,
	.TxFn = Pms_Tx,
};

/*电池槽位1的命令配置表*******************************************************************/
//const static 
	ModCmd g_Bms1Cmds[BMS_CMD_COUNT] =
{
	{&g_BmsCmdEx[0], BMS_WRITE_CTRL, MOD_WRITE,MOD_WEITE_SINGLE_REG,"WriteReg1-Ctrl", &g_Bat[1].writeBmsCtrl, 2, (void*)g_bmsCtrl_writeParam, 2,  &g_Bat[1].bmsCtrl},
	{&g_BmsCmdEx[1], BMS_READ_ID    ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1ID"   , &g_Bat[1].bmsID		 , BMS_REG_ID_SIZE    , (void*)g_bmsID_readParam, 4,Null,(ModEventFn)Bat_event_readBmsID},
	{&g_BmsCmdEx[2], BMS_READ_INFO_1,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info1", &g_Bat[1].bmsInfo	     , BMS_REG_INFO_SIZE_1, (void*)g_bmsInfo1_readParam, 4, Null, (ModEventFn)Bat_event_readBmsInfo},
	{&g_BmsCmdEx[3], BMS_READ_INFO_2,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Info2", &g_Bat[1].bmsInfo.cmost, BMS_REG_INFO_SIZE_2, (void*)g_bmsInfo2_readParam,4,Null,(ModEventFn)Bat_event_readBmsInfo2},
	{&g_BmsCmdEx[4], BMS_READ_CTRL  ,MOD_READ, MOD_READ_HOLDING_REG, "ReadBms1Ctrl" , &g_Bat[1].bmsCtrl	     , BMS_REG_CTRL_SIZE  , (void*)g_bmsCtrl_readParam, 4},
	
};
//const 
static ModCfg g_Bat1Cfg =
{
	.port = 1,
	.antselct = 1,
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



void BatteryInfoDump(void)
{
	int i = 0;
	Battery* pPkt = Null;

	Printf("Battery info:\n");
//	BAT_DUMP(batteryCount);
	for(i = 0; i < MAX_BAT_COUNT; i++)
	{
		pPkt = &g_Bat[i];
		if(pPkt->presentStatus == BAT_IN)
		{
			Bat_bmsInfoDump(pPkt);
		}
	}
}

void BatteryDump2(void)
{
	int i = 0;
	Battery* pPkt = Null;

	Printf("Battery info:\n");
	Printf("NFC:%d:%d\n",BMS_REG_INFO_ADDR_1,BMS_REG_INFO_COUNT_1);
	Printf("NFC:%d:%d\n",BMS_REG_INFO_ADDR_2+1,BMS_REG_INFO_COUNT_2);
//	BAT_DUMP(batteryCount);
	for(i = 0; i < MAX_BAT_COUNT; i++)
	{
		pPkt = &g_Bat[i];
		if(pPkt->presentStatus == BAT_IN)
		{
			Bat_bmsInfoDump2(pPkt);
		}
	}
}


void BatteryDump(void)
{
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		Bat_dump(&g_Bat[i]);
	}
}

void NfcCardReaderDump()
{
	#define PRINTF_NFC(_field) Printf("\t%s=%d\n", #_field, g_pms.fmDrv._field);
	
	Printf("NFC info:\n");
	
	PRINTF_NFC(state);
	PRINTF_NFC(antPort);
}

void PmsDump()
{
	#define PRINTF_PMS(_field) Printf("\t%s=%d\n", #_field, g_pms._field);
	
	Printf("Pms info:\n");
	
	PRINTF_PMS(opStatus);
	PRINTF_PMS(statusSwitchTicks);
}

/********************************************************************/

static Pms_FsmFn Pms_findStatusProcFun(PmsOpStatus status);

//void Pms_setPreDischg(Bool en)	{ g_regCtrl.preDischg   = en; Bat_setPreDischg(g_pActiveBat, en); }
//void Pms_setDischg(Bool en)		{ g_regCtrl.dischgEn    = en; Bat_setDischg(g_pActiveBat, en); }
//void Pms_setChg(Bool en)		{ g_regCtrl.chgEn       = en; Bat_setChg(g_pActiveBat, en); }
//void Pms_setSleep(Bool en)		{ g_regCtrl.sleepEn     = en; Bat_setSleep(g_pActiveBat, en); }
//void Pms_setDeepSleep(Bool en)	{ g_regCtrl.deepSleepEn = en; Bat_setDeepSleep(g_pActiveBat, en); }

static int Pms_Tx(const uint8_t* pData, int len)
{
	//调用NFC发送数据命令
	fm175Drv_transferInit(&g_pms.fmDrv, pData, len, g_pModBus->frameCfg->rxBuf, g_pModBus->frameCfg->rxBufLen, g_pActiveBat);
	return len;
}

//从总线接收到的数据，调用该函数处理
void Pms_Rx(Battery* pBat, const uint8_t* pData, int len)
{
	//从NFC驱动接收数据
//	if (g_pActiveBat->port != nfcPort) return;

	Mod_RxData(g_pModBus, pData, len);
}

void Pms_postMsg(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	Message* msg = Queue_getNew(&g_pms.msgQueue);
	if (msg)
	{
		msg->m_MsgID = msgId;
		msg->m_Param1 = param1;
		msg->m_Param2 = param2;
	}
	else
	{
		PFL_WARNING("Pms msg queue is full.\n");
	}
}

void Pms_plugIn(Battery* pBat)
{
	Bat_msg(pBat, BmsMsg_batPlugIn, 0, 0);
}

void Pms_plugOut(Battery* pBat)
{
	//Battery* pBat = (port == 0) ? &g_Bat[1] : &g_Bat[0];

	Bat_msg(pBat, BmsMsg_batPlugout, 0, 0);
	*((uint16*)& g_regCtrl) = 0;
/*
	if(g_Bat[0].presentStatus != BAT_NOT_IN )
	{
		*((uint16*)& g_regCtrl[0]) = 0;
	}
*/	
}

void Pms_SwitchPort()
{
	Battery* pBat = (g_pActiveBat->port == 0) ? &g_Bat[1] : &g_Bat[0];

	if (!Mod_isIdle(g_pModBus)) return;
//	Fsm_SetActiveFlag(AF_MDB,False);
	rt_thread_mdelay(500);
//	Fsm_SetActiveFlag(AF_MDB,True);
	if(!(Fsm_ReadActiveFlag() & AF_PMS))
	{
		fm175Drv_stop(&g_pms.fmDrv);
	}
	Bat_msg(g_pActiveBat, BmsMsg_deactive, 0, 0);
	
	Mod_SwitchCfg(g_pModBus, (g_pActiveBat->port == 0) ? &g_Bat1Cfg : &g_Bat0Cfg);
	fm175Drv_switchNfc(&g_pms.fmDrv, (g_pActiveBat->port == 0) ? FM17522_I2C_ADDR1 : FM17522_I2C_ADDR);
	Bat_msg(pBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
	//Bat_msg(g_pActiveBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
	
	g_pActiveBat = pBat;

	PFL(DL_PMS,"NFC Switch Bat[%d]:%x\n",g_pActiveBat->port,g_pms.fmDrv.iicReg.dev_addr );
}

PmsOpStatus Pms_GetStatus(void)
{
	return g_pms.opStatus;
}

//static 
void Pms_switchStatus(PmsOpStatus newStatus)
{
	if (newStatus == g_pms.opStatus) return;

	g_pms.statusSwitchTicks = GET_TICKS();

	if (newStatus == PMS_ACC_OFF)
	{
//		Pms_setDischg(True);
//		Pms_setChg(True);
	}
	else if (newStatus == PMS_ACC_ON)
	{
//		Pms_setDischg(True);
//		Pms_setChg(True);
	}
	else if (newStatus == PMS_SLEEP)
	{
//		Pms_setDischg(True);
	}
	else if (newStatus == PMS_DEEP_SLEEP)
	{
//		Pms_setDischg(False);
//		Pms_setChg(False);
	}
	PFL(DL_PMS,"pms status from %d to %d\n",g_pms.opStatus,newStatus);
	g_pms.opStatus = newStatus;
	g_pms.Fsm = Pms_findStatusProcFun(newStatus);
}

/*
	acc off 时，不关电，只关锁
	如果电池未充电时，则等待30s进入deepsleep
	如果正在充电,则暂时不进入deepsleep
*/
static void Pms_fsm_accOff(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	if (msgId == PmsMsg_run)
	{
		//static uint32 accoff_tick[MAX_BAT_COUNT] = 0 ;
		static uint32 accoffprintf_tick = 0;
		//uint8 i = 0 ;
		if ((0/*g_pJt->devState.cnt & _GPS_FIXE_BIT*/)||(SwTimer_isTimerOutEx(g_pms.statusSwitchTicks,PMS_ACC_OFF_ACTIVE_TIME)))
		{
			// 不客是否低电量,都要进休眠
			//18650电压高或者电池不在位，进入休眠模式	
			// 如果正在充电,则暂时不要进入deepsleep
			
			if( g_pdoInfo.isLowPow && 
				( ( g_Bat[0].presentStatus == BAT_IN && g_Bat[0].bmsInfo.state & 0x0200 ) ||
					g_Bat[1].presentStatus == BAT_IN && g_Bat[1].bmsInfo.state & 0x0200 ) )
			{
				// 暂时不进deepsleep
				PFL(DL_PMS,"Battery Low...\n");
			}
			else
			{
				Pms_switchStatus(PMS_DEEP_SLEEP);
			}
		}
		//
		if( GET_TICKS() - accoffprintf_tick > 1000 )
		{
			Battery_discharge_process();
			accoffprintf_tick = GET_TICKS();
		}
		// 锁车
		PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, True);
		g_pdoInfo.isWheelLock = 1;	// 轮毂锁 锁

	}
	else if (msgId == PmsMsg_accOn)
	{
		Pms_switchStatus(PMS_ACC_ON);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{
		Pms_plugIn((Battery*)param1);
#ifndef _GENERAL_CENTRAL_CTL	// 普通中控,检测到电池接入则放电，非普通中控 	
		if( g_pdoInfo.isRemoteAccOn )
#endif //			
			Pms_switchStatus(PMS_ACC_ON);
	}
	else if (msgId == PmsMsg_batPlugOut)
	{
		Pms_plugOut((Battery*)param1);
	}
	else if (msgId == PmsMsg_GyroIrq)
	{
		PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, True);
		g_pdoInfo.isWheelLock = 1;			// 锁车
		g_pms.statusSwitchTicks = GET_TICKS();
	}
	else if( msgId == PmsMsg_18650Low )
	{
		//如果当前没有acc ON 则启动放电
		PFL(DL_PMS,"18650 Low ,Request Charging\n");
	}
	else if( msgId == PmsMsg_18650Normal )
	{
		//如果当前没有acc on 则关闭放电
		PFL(DL_PMS,"18650 Normal ,Request to stop charging\n");
	}
#ifdef CANBUS_MODE_JT808_ENABLE
	else if( msgId == PmsMsg_GPRSIrq )
	{
		g_pms.statusSwitchTicks = GET_TICKS();		// 刷新时间
	}
#endif //
}

static void Pms_fsm_accOn(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	static uint32 accoonprintf_tick = 0;
	static uint32 discarge_tick = 0 ;
	if (msgId == PmsMsg_run)
	{
		if( GET_TICKS() - accoonprintf_tick > 1000 )
		{
			Battery_discharge_process(); 
			accoonprintf_tick = GET_TICKS();
		}
		// 检测到放电电流<2A则进入accoff
		// 
	}
	else if (msgId == PmsMsg_accOff)
	{
		Pms_switchStatus(PMS_ACC_OFF);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{
		Pms_plugIn((Battery*)param1);
	}
	else if (msgId == PmsMsg_batPlugOut)
	{
		Pms_plugOut((Battery*)param1);
		if( g_Bat[0].presentStatus != BAT_IN && g_Bat[1].presentStatus != BAT_IN )
		{
			Pms_switchStatus(PMS_ACC_OFF);
		}
	}
	else if( msgId == PmsMsg_18650Low )
	{
		PFL(DL_PMS,"18650 Low ,Request Charging\n");
	}
	else if( msgId == PmsMsg_18650Normal )
	{
		PFL(DL_PMS,"18650 Normal ,Request to stop charging\n");
	}
	PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, False);
	g_pdoInfo.isWheelLock = 0;		// 轮毂锁开
}

static void Pms_fsm_sleep(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_CANErr(PmsMsg msgId, uint32_t param1, uint32_t param2)
{

}

static void Pms_fsm_deepSleep(PmsMsg msgId, uint32_t param1, uint32_t param2)
{
	static uint32 accsleep_tick = 0;
	static uint32 accoffprintf_tick = 0;
	static uint32 accoff_tick[MAX_BAT_COUNT];
	uint8 i = 0;
	// 开锁，防止费电，关闭放电
	PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, False);
	g_pdoInfo.isWheelLock = 1;

	//	if(((g_Bat[0].bmsInfo.state&0x0300)!=0x0000)&&((GET_TICKS() -accsleep_tick) >3000 ))
	//	{
	//		Pms_setDischg(False);
	//		Pms_setChg(False);
	//		accsleep_tick = GET_TICKS();
	//	}
	//	if((g_Bat[0].bmsInfo.state&0x0300)==0x0000)
	//	{
	//		Fsm_SetActiveFlag(AF_PMS, False);
	//	}

	
	if (SwTimer_isTimerOutEx(g_pms.statusSwitchTicks,PMS_ACC_DEEPSLEEP_TIME))
	{
		workmode_switchStatus(WM_SLEEP);
	}
	
	if (msgId == PmsMsg_run)
	{
		for(  i = 0 ; i < MAX_BAT_COUNT ; i++ )
		{
			if( g_Bat[i].presentStatus == BAT_IN && 
				(g_Bat[i].bmsInfo.state & 0x0300) != 0x00 &&
				GET_TICKS() - accoff_tick[i] > 3000 )
			{
				Bat_setDischg(&g_Bat[i], False);
				Bat_setChg(&g_Bat[i], False);
				accoff_tick[i] = GET_TICKS();
				PFL(DL_PMS,"ACC Off Bat[%d]:0x%04X Dischg&Chg\n",i,g_Bat[i].bmsInfo.state);
			}
		}
		//
	}
	if (msgId == PmsMsg_GyroIrq)
	{
		PortPin_Set(g_pLockEnIO->periph, g_pLockEnIO->pin, True);
		g_pdoInfo.isWheelLock =1;
		Pms_switchStatus(PMS_ACC_OFF);
	}
	else if (msgId == PmsMsg_accOn)
	{
		if((g_pdoInfo.isRemoteAccOn)&&
			(g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN))
			Pms_switchStatus(PMS_ACC_ON);
	}
	else if (msgId == PmsMsg_batPlugIn)
	{
		Pms_plugIn((Battery*)param1);
#ifndef _GENERAL_CENTRAL_CTL	// 普通中控,检测到电池接入则放电，非普通中控 	
		if(g_pdoInfo.isRemoteAccOn )
#endif //			
			Pms_switchStatus(PMS_ACC_ON);
	}
#ifdef CANBUS_MODE_JT808_ENABLE
	else if( msgId == PmsMsg_GPRSIrq )	// 不要休眠
	{
		g_pms.statusSwitchTicks = GET_TICKS();
	}
#endif //

}

//在任何状态下都要处理的消息函数
static void Pms_fsm_anyStatusDo(PmsMsg msgId, uint32_t param1, uint32_t param2)
{	
//	int i = 0;
//	Battery* pPkt = Null;

//	for(i = 0; i < 2; i++)
//	{
//		pPkt = &g_Bat[i];
//		
//		if(Bat_isReady(pPkt))
//		{
//			if((pPkt->bmsInfo.state&0x01)!= ())
//			{
//			
//			}
//		}
//	}
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

//void Pms_cardReaderReadFifo(void)
//{
//	rt_interrupt_enter();
//	NfcCardReader_read_fifo(&g_pms.cardReader);
//	PFL(DL_NFC,"NFC IRQ haddle!\n");
//	rt_interrupt_leave();
//}

TRANSFER_EVENT_RC Pms_EventCb(Battery* pBat, TRANS_EVENT ev)
{
	Fm175Drv* fmDrv = &g_pms.fmDrv;
	TransParam* param = &fmDrv->transParam;
	if (ev == SEARCH_CARD_DONE)
	{
		if (fmDrv->cardIsExist)
		{
			
			PFL(DL_NFC, "Bat[0x%x] NFC port[%d] search success!\n", fmDrv->iicReg.dev_addr,((Battery*)fmDrv->obj)->cfg->antselct);
		}
		else
		{
			
			PFL(DL_NFC, "Bat[0x%x] NFC port[%d] search failed!\n", fmDrv->iicReg.dev_addr,((Battery*)fmDrv->obj)->cfg->antselct);
		}
	}
	else if (ev == TRANS_SUCCESS)
	{
		Pms_Rx(pBat, param->rxBuf, param->totalLen);
		PFL(DL_NFC,"Bat[0x%x] NFC port[%d] rx date success!\n", fmDrv->iicReg.dev_addr,((Battery*)fmDrv->obj)->cfg->antselct);
	}
	else if (ev == TRANS_FAILED)
	{
		Mod_busErr(g_pModBus, BUS_ERR_RX_FAILED);
		PFL(DL_NFC,"Bat[0x%x] NFC port[%d] rx date failed!\n", fmDrv->iicReg.dev_addr,((Battery*)fmDrv->obj)->cfg->antselct);
	}

	return TRANS_EVT_RC_SUCCESS;
}

void Pms_stop()
{
	Pms_switchStatus(PMS_DEEP_SLEEP);
	//fm175Drv_stop(&g_pms.fmDrv);
}	

void Pms_run()
{
	fm175Drv_run(&g_pms.fmDrv);

	Mod_Run(g_pModBus);
	#ifdef DGT_CONFIG
	WDOG_Feed();
	#endif
	Bat_run(&g_Bat[0]);
	Bat_run(&g_Bat[1]);

	Pms_fsm(PmsMsg_run, 0, 0);

	//消息循环
	Message* msg = (Message*)Queue_Read(&g_pms.msgQueue);
	for (; msg != Null; Queue_pop(&g_pms.msgQueue), msg = (Message*)Queue_Read(&g_pms.msgQueue))
	{
		Pms_fsm(msg->m_MsgID, msg->m_Param1, msg->m_Param2);
	}
}

void Pms_start()
{
#ifdef _GENERAL_CENTRAL_CTL
	if(g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN )
#else
	if((g_pdoInfo.isRemoteAccOn)&&(
		g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN))
#endif //		
	{
		Pms_switchStatus(PMS_ACC_ON);
		PMS_DEBUG_MSG("PMS Start AccOff\n");
	}
	else
	{
		Pms_switchStatus(PMS_ACC_OFF);
		PMS_DEBUG_MSG("PMS Start AccOff\n");
	}
	g_pms.statusSwitchTicks = GET_TICKS();
	//查询电池设备信息
	Bat_msg(g_pActiveBat, BmsMsg_active, *((uint32*)& g_regCtrl), 0);
	Bat_start(&g_Bat[0]);
	fm175Drv_start(&g_pms.fmDrv);
	Fsm_SetActiveFlag(AF_PMS, True);
}

void Pms_init()
{
	static const TransProtocolCfg fm175DrvCfg =
	{
		.fifoDeepth = 64,
	};
	static const Obj obj = { "Pms", Pms_start, Pms_stop, Pms_run };
	ObjList_add(&obj);

	Mod_Init(g_pModBus, &g_Bat0Cfg, &g_frameCfg);
	fm175Drv_init(&g_pms.fmDrv, FM17522_I2C_ADDR, &fm175DrvCfg, (TransEventFn)Pms_EventCb);

	Bat_init(&g_Bat[0], 0, &g_Bat0Cfg);
	Bat_init(&g_Bat[1], 1, &g_Bat1Cfg);

	g_pActiveBat = &g_Bat[0];
	Queue_init(&g_pms.msgQueue, g_pms.msgBuf, sizeof(Message), GET_ELEMENT_COUNT(g_pms.msgBuf));
	g_pLockEnIO = IO_Get(IO_LOCK_EN);
}
