
#include "rs485_protocol.h"
#include "common.h"
#include "drv_usart.h"
#include "Rs485Modbus.h"

Mod g_Rs485ModBus;
uint32_t Rs485RunTicks;
uint8_t Rs485TestSendFlag =0;
uint8_t Rs485TestRxBuff[10];
uint8_t Rs485TestTxBuff[5]={0x01,0x02,0x03,0x04,0x05};

//所有的BMS单个命令的事件回调函数
MOD_EVENT_RC Rs485Test_event(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		if((pCmd->cmd == 1)&&( pCmd->pExt->transferLen == sizeof(Rs485TestTxBuff)))
		{
			if (memcmp(pCmd->pStorage, pCmd->pExt->transferData, pCmd->pExt->transferLen) == 0)
			{
				g_pdoInfo.isRs485Ok = 1;
				Rs485TestSendFlag =1;
			}
		}
	}
	else if (ev == MOD_REQ_FAILED)
	{
		g_pdoInfo.isRs485Ok = 0;
		Rs485TestSendFlag =0;
	}

	return MOD_EVENT_RC_SUCCESS;
}


int Rs485_Tx(const uint8_t* pData, int len)
{
	int i;
	DrvIo* g_Rs485DirCtrlIO = Null;
	
	g_Rs485DirCtrlIO = IO_Get(IO_DIR485_CTRL);
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, True);
	for(i=0;i<len;i++)
	{
		uart4_put_byte(*pData++);
	}
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, False);
	
	return 0;
}

int Rs485RxData(void)
{
	char data;
	
	data = rs485_getchar();
	if (data == 0xff)
	{

	}
	else
	{
		Rs485Mod_RxData(&g_Rs485ModBus,&data,1);
	}
}

void RS485_Run(void)
{
	const uint16_t Rs485Sendms = 1000;
	Rs485RxData();
	Rs485Mod_Run(&g_Rs485ModBus);
	if((SwTimer_isTimerOutEx(Rs485RunTicks,Rs485Sendms))&&
		(Rs485TestSendFlag == 0))
	{
		Rs485RunTicks = GET_TICKS();
		Mod_SendCmd(g_Rs485ModBus.cfg, 1);
	}
}

void RS485_Start(void)
{
	Rs485RunTicks = GET_TICKS();
	Rs485TestSendFlag =0;
}

void RS485_Init(void)
{
	#define RS485_CMD_COUNT 5
	static ModCmdEx g_BmsCmdEx[RS485_CMD_COUNT];
	//ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
	/*电池槽位0的命令配置表*******************************************************************/
	const static ModCmd g_Rs4850Cmds[RS485_CMD_COUNT] =
	{
		{&g_BmsCmdEx[0], 1 ,MOD_READ, MOD_READ_HOLDING_REG, "Rs485Test" , Rs485TestRxBuff , sizeof(Rs485TestRxBuff) ,
        	Rs485TestTxBuff, sizeof(Rs485TestTxBuff), Null,(ModEventFn)Rs485Test_event},
		
	};

	const static ModCfg Rs485Cfg =
	{
		.port = 0,
		.cmdCount = RS485_CMD_COUNT,
		.cmdArray = g_Rs4850Cmds,
//		.pCbObj = &g_Bat[0],
//		.TresferEvent = (ModEventFn)Bat_event,
		.TxFn = Rs485_Tx,
	};
	/*Modbus传输协议帧配置*****************************************************/
	static uint8_t g_Rs485modBusTxBuf[128];
	static uint8_t g_Rs485modBusRxBuf[128];
	const static ModFrameCfg g_Rs485frameCfg =
	{
		.txBuf = g_Rs485modBusTxBuf,
		.txBufLen = sizeof(g_Rs485modBusTxBuf),
		.rxBuf = g_Rs485modBusRxBuf,
		.rxBufLen = sizeof(g_Rs485modBusRxBuf),

		.waitRspMsDefault = 1000,
		.rxIntervalMs = 1,
		.sendCmdIntervalMs = 100,
	};
	
	const static Obj obj = {
	.name = "RS485",
	.Start = RS485_Start,
	.Run = RS485_Run,
	};

	ObjList_add(&obj);
	
	Rs485Mod_Init(&g_Rs485ModBus, &Rs485Cfg, &g_Rs485frameCfg);
	
	
}
