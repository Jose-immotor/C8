 /*******************************************************************************
 * Copyright (C) 2020 by IMMOTOR
 *
 * File	  : main.c
 * 
 * 更新电池信息
 *
 * Change Logs:
 * Date		      Author		Notes
 * 2020-05-25	   lane 	first implementation
*******************************************************************************/
#include <rtthread.h>
#include "Common.h"
#include "pms.h"
#include "fm175xx.h"
#include "log.h"
#include "Smart_System.h"
#include "nvc.h"
#include "nfc_cmd_process.h"
#include "battery.h"
#include "fsm.h"

//#include "adc_sample.h"

#define PLUG_IN  1
#define PLUG_OUT 0
//#define MAX_DEACTIVE_ALLOW_TIME 180000

//static Utp __g_Utp;
//Utp* g_pUtp = &__g_Utp;

static Pms __g_Pms;
Pms* g_pPms = &__g_Pms;

//static SwTimer g_PmsFwUpgradeTimer;
//SwTimer g_PmsPwrOffTimer;
//SwTimer g_PmsAccOffTimer;

//static uint32 g_DeactiveTimer = 0;
//static uint32 g_DeactiveBeepFlag = 0;

st_base_param gl_base_param[NFC_READER_COUNT_MAX];

//static struct rt_event     pms_event;
//bool pmsPortEventInit( void )
//{
////	rt_event_init(&pms_event,"pms event",RT_IPC_FLAG_PRIO);
//    return TRUE;
//}

//BOOL pmsPortEventPost( pmsEventType eEvent )
//{
//    rt_event_send(&pms_event, eEvent);
//    return TRUE;
//}

//BOOL pmsPortEventGet( pmsEventType * eEvent )
//{
//    rt_uint32_t recvedEvent=0;
//    /* waiting forever OS event */
//    rt_event_recv(&pms_event,
//            EV_PMS_ONLINE | EV_MASTER_FRAME_RECEIVED | EV_MASTER_EXECUTE |
//            EV_MASTER_FRAME_SENT | EV_MASTER_ERROR_PROCESS,
//            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_NO,
//            &recvedEvent);
//    /* the enum type couldn't convert to int type */
//    switch (recvedEvent)
//    {
//    case EV_PMS_ONLINE:
//        *eEvent = EV_PMS_ONLINE;
//        break;
//    case EV_MASTER_FRAME_RECEIVED:
//        *eEvent = EV_MASTER_FRAME_RECEIVED;
//        break;
//    case EV_MASTER_EXECUTE:
//        *eEvent = EV_MASTER_EXECUTE;
//        break;
//    case EV_MASTER_FRAME_SENT:
//        *eEvent = EV_MASTER_FRAME_SENT;
//        break;
//    case EV_MASTER_ERROR_PROCESS:
//        *eEvent = EV_MASTER_ERROR_PROCESS;
//        break;
//    }
//    return TRUE;
//}
//Bool Pms_IsFwUpgrade()
//{
//	return g_PmsFwUpgradeTimer.m_isStart;
//}
static int16 pms_get_max_output_current(void)
{
    st_modbus_reg_unit vl_tmp;
    unsigned short vl_tmp_value;
    
    vl_tmp.bits16_H = 0;
    vl_tmp.bits16_L = 0;
    //if(gl_bms_info_p[0]->reg_value_ready)
    if(slave_rs485_is_bat_valid(0))
    {
        vl_tmp = gl_bms_info_p[0]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_BCAP)];
    }

    vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);

    vl_tmp.bits16_H = 0;
    vl_tmp.bits16_L = 0;
    //if(gl_bms_info_p[1]->reg_value_ready)
    if(slave_rs485_is_bat_valid(1))
    {
        vl_tmp = gl_bms_info_p[1]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_BCAP)];
    }

    vl_tmp_value += ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);

    vl_tmp_value /=10;
    vl_tmp_value *=3;

	return vl_tmp_value;

}
static uint16 pms_get_work_voltate(void)
{
    st_modbus_reg_unit vl_tmp;
    unsigned short vl_tmp_value;
    
    vl_tmp.bits16_H = 0;
    vl_tmp.bits16_L = 0;

    if(slave_rs485_is_bat_valid(0))
    {
        vl_tmp = gl_bms_info_p[0]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TVOLT)];
        
    }
    else if(slave_rs485_is_bat_valid(1))
    {
        vl_tmp = gl_bms_info_p[1]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TVOLT)];
    }

    vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);

    return vl_tmp_value;
}

static int16 pms_get_work_current(void)
{
    st_modbus_reg_unit vl_tmp;
    int vl_tmp_value;
    int vl_real_value;
    unsigned short vl_u16_tmp;
    
    vl_tmp_value = 0;
    vl_real_value = 0;
    
    vl_tmp.bits16_H = 0xFF;
    vl_tmp.bits16_L = 0xFF;
    //if(gl_bms_info_p[0]->reg_value_ready)
    if(slave_rs485_is_bat_valid(0))
    {
        vl_tmp = gl_bms_info_p[0]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR)];
    }
    if((vl_tmp.bits16_H != 0xFF)&&(vl_tmp.bits16_L != 0xFF))
    {
        vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
        vl_real_value = vl_tmp_value-300*100;
    }

    vl_tmp.bits16_H = 0xFF;
    vl_tmp.bits16_L = 0xFF;
    //if(gl_bms_info_p[1]->reg_value_ready)
    if(slave_rs485_is_bat_valid(1))
    {
        vl_tmp = gl_bms_info_p[1]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR)];
    }

    if((vl_tmp.bits16_H != 0xFF)&&(vl_tmp.bits16_L != 0xFF))
    {
        vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
        vl_real_value += (vl_tmp_value-300*100);
    }

    vl_real_value += (300*100);
    vl_u16_tmp = vl_real_value;

    return vl_u16_tmp;
}

uint8 Pms_GetAveBatSoc(const BatStatePkt* pBatDesc)
{
	int i = 0;
	uint8 soc = 0;

	if(pBatDesc == Null)
	{
		pBatDesc = &g_pPms->m_batPkt;
	}

	//Get Soc
	for(i = 0; i < NFC_READER_COUNT_MAX; i++)
	{
		if(pBatDesc->desc[i].capacity)
		{
			soc += pBatDesc->desc[i].SOC;
		}
	}

	return pBatDesc->batteryCount ? soc / pBatDesc->batteryCount : 0;
}


//#define PRINTF_SERIAL(str, pSerialNum){const uint8* p = (pSerialNum); Printf("%s[%02X%02X%02X%02X%02X%02X]\n",str, p[0], p[1], p[2], p[3], p[4], p[5]);}
void BatteryDescDump(const BatteryDesc* desc)
{
	#define BAT_DUMP1(_v) Printf("\t\t%s=0x%x(%d)\n", #_v, desc->_v, desc->_v)
	const uint8* pByte = desc->serialNum;
	Printf("\tPort[%d][%02X%02X%02X%02X%02X%02X]:\n"
		,desc->portId, pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5]);
	
	BAT_DUMP1(SOC);
	BAT_DUMP1(voltage);
	BAT_DUMP1(current);
//	BAT_DUMP1(temp);
//	BAT_DUMP1(fault);
//	BAT_DUMP1(damage);
//	BAT_DUMP1(cycleCount);
//	Printf("\t\tmaxVoltage=%d,%d\n", desc->maxVoltage, desc->maxVolCell);
//	Printf("\t\tminVoltage=%d,%d\n", desc->minVoltage, desc->minVolCell);
//	BAT_DUMP1(bmsPcbTemp);
//	BAT_DUMP1(connectorTemp);
	BAT_DUMP1(mosState);
}

void BatteryDump(const BatStatePkt* pPkt)
{
//    VerDesc* pVerDesc = GetPmsVerDesc();
	
	if(Null == pPkt) pPkt = &g_pPms->m_batPkt;
	int i = 0;
	#define BAT_DUMP(_v) Printf("\t%s=0x%x(%d)\n", #_v, pPkt->_v, pPkt->_v)
	#define PMS_DUMP(_v) Printf("\t%s=0x%x(%d)\n", #_v, g_pPms->_v, g_pPms->_v)
	
//	for(int i = 0; i < BAT_SERIALNUM_COUNT; i++)
//	{
//		PRINTF_SERIAL("\t", &g_SysCfg.SerialNums[i][0]);
//	}
	
//	VerDesc_Dump(pVerDesc, "\tPms ");

//	PMS_DUMP(m_isNotDischarge);
//	PMS_DUMP(m_PowerOffAtOnce);
	
	Printf("Battery info:\n");
	
//	BAT_DUMP(MaxOutPutCurrent);
//	BAT_DUMP(workVoltate);
//	BAT_DUMP(workCurrent);
//	BAT_DUMP(packState);
	BAT_DUMP(batteryCount);
//	BAT_DUMP(deviceState);
	for(i = 0; i < NFC_READER_COUNT_MAX; i++)
	{
		if(pPkt->desc[i].capacity)
		{
		//	BmsFault* pBmsFault;
		//  char buf[20];
			BatteryDescDump(&pPkt->desc[i]);
			//pBmsFault = &g_pPms->m_BmsFault[i];
			//Printf("\t\tdevFt1=0x%x[%s]\n", pBmsFault->devFt1, ToBinStr(&pBmsFault->devFt1, 2, buf));
			//Printf("\t\tdevFt2=0x%x[%s]\n", pBmsFault->devFt2, ToBinStr(&pBmsFault->devFt2, 2, buf));
			//Printf("\t\topFt1=0x%x[%s]\n" , pBmsFault->opFt1, ToBinStr(&pBmsFault->opFt1  , 2, buf));
			//Printf("\t\topFt2=0x%x[%s]\n" , pBmsFault->opFt2, ToBinStr(&pBmsFault->opFt2  , 2, buf));
		}
	}
}



////Bool Battery_isFault(const BatteryDesc* pDesc)
////{
//////	return pDesc->fault != 0;

////	//只有欠压才算故障，其他故障不需要处理
////	BatFault* pBatFault = (BatFault*)&pDesc->fault;
////	return pBatFault->UVP;
////}

//Bool Battery_isIn(uint8 port)
//{
//	return (slave_rs485_is_bat_valid(port));//(g_pPms->m_portMask & (1 << port)) != 0 ;
//}

////static Bool Battery_isErr(BmsFault* pBmsFault)
////{
////	return (pBmsFault->devFt1 & DEVFT1_ALARM_MASK
////		|| pBmsFault->devFt2 & DEVFT2_ALARM_MASK
////		|| pBmsFault->opFt1 & OPFT1_ALARM_MASK
////		|| pBmsFault->opFt2 & OPFT2_ALARM_MASK
////		);
////}

////static uint8 Pms_GetPortFault(const BatteryDesc* pDesc)
////{
////	uint8 flag = 0;	
////	//BatteryDesc* pDesc = &g_pPms->m_batPkt.desc[port];
////	BatFault* pBatFault = (BatFault*)&pDesc->fault;
////	
////	if(pBatFault->OVP && pDesc->maxVoltage >= 425)
////	{
////		flag |= BIT_0;	//过压，非法充电
////	}
////	if(pBatFault->OTP)
////	{
////		flag |= BIT_1;	//过温
////	}
////	return flag;
////}
/////*
////Bool Bms_isErr()
////{
////	for(int i = 0; i < MAX_BATTERY_COUNT; i++)
////	{
////		if(Battery_isIn(i))
////		{
////			if(Battery_isErr(&g_pPms->m_BmsFault[i]) || Pms_GetPortFault(&g_pPms->m_batPkt.desc[i]))
////			{
////				return True;
////			}
////		}
////	}
////	
////	return False;
////}
////*/
////Bool Battery_SerialNumIsSame(const uint8* pSerialNum1, const uint8* pSerialNum12)
////{
////	//return True;
////	
////	int i = 0;
////	uint8 temp[SERIAL_NUM_SIZE];
////	for(i = 0; i < SERIAL_NUM_SIZE; i++)
////	{
////		temp[i] = pSerialNum1[SERIAL_NUM_SIZE - i - 1];
////	}
////	
////	return ((memcmp(pSerialNum1, pSerialNum12, SERIAL_NUM_SIZE) == 0)
////		|| (memcmp(temp, pSerialNum12, SERIAL_NUM_SIZE) == 0));
////	
////}

//void Pms_RemoveAllSn(uint8 count)
//{
//	#if 0
//	int i = 0;
//	count = (count > BAT_SERIALNUM_COUNT) ? BAT_SERIALNUM_COUNT : count;
//	for(i = 0; i < count; i++)
//	{
//		memset(&g_SysCfg.SerialNums[i][0], 0, SERIAL_NUM_SIZE);
//	}
//	#endif
//}

////uint8* Pms_FindSn(const uint8* pSn)
////{
////	#if 0
////	for(int i = 0; i < BAT_SERIALNUM_COUNT; i++)
////	{
////		if(memcmp(pSn, &g_SysCfg.SerialNums[i][0], SERIAL_NUM_SIZE) == 0)
////		{
////			return &g_SysCfg.SerialNums[i][0];
////		}
////	}
////	#endif
////	return Null;
////}

//Bool Pms_RemoveSn(const uint8* pSn)
//{
//	#if 0
//	for(int i = 0; i < BAT_SERIALNUM_COUNT; i++)
//	{
//		if(memcmp(pSn, &g_SysCfg.SerialNums[i][0], SERIAL_NUM_SIZE) == 0)
//		{
//			for(int j = i; j < BAT_SERIALNUM_COUNT-1; j++)
//			{
//				memcpy(&g_SysCfg.SerialNums[j][0], &g_SysCfg.SerialNums[j+1][0], SERIAL_NUM_SIZE);
//			}
//			
//			memset(&g_SysCfg.SerialNums[BAT_SERIALNUM_COUNT-1][0], 0, SERIAL_NUM_SIZE);
//			return True;
//		}
//	}	
//	#endif
//	return False;
//}

//Bool Pms_AddSn(const uint8* pSn)
//{
//	#if 0
//	int i = 0;
//	static const uint8 g_EmptySerial[SERIAL_NUM_SIZE] = {0};

//	//已经存在
//	if(Pms_FindSn(pSn))
//	{
//		return False;
//	}

//	//寻找空位
//	uint8* pSerialNum = Pms_FindSn(g_EmptySerial);
//	if(pSerialNum)
//	{
//		memcpy(pSerialNum, pSn, SERIAL_NUM_SIZE);
//		return True;
//	}

//	//现存所有位置下移
//	for(i = 0; i < BAT_SERIALNUM_COUNT-1; i++)
//	{
//		memcpy(&g_SysCfg.SerialNums[i][0], &g_SysCfg.SerialNums[i+1][0], SERIAL_NUM_SIZE);
//	}

//	//新增的内容放在最高位
//	memcpy(&g_SysCfg.SerialNums[BAT_SERIALNUM_COUNT-1][0], pSn, SERIAL_NUM_SIZE);
//	return True;
//	#else
//	return False;
//	#endif
//}


//////检查PMS硬件版本号是否有效，如果无效，说明被非法擦除, 则从备份区恢复PMS固件
////void Pms_CheckFw()
////{
////	extern uint32 File_CalcuCrc(uint32 pAddr, int len);
////	
////	if(g_Settings.pmsVer.m_HwMainVer != 0 || g_Settings.pmsVer.m_HwSubVer != 0) return ;	//有效

////	int i = 0;
////	Bool isFound = False;
////	CrcSector crcSector;
////	uint32 addr = (uint32)g_pNvdsMap->pmsFileBuf;
////	
////	memset(&g_CommonBuf[BIN_SECTOR_SIZE], 0xFF, BIN_SECTOR_SIZE);
////	
////	for(i = 0; i < FILE_BUF_SIZE; i += BIN_SECTOR_SIZE)
////	{
////		Mx25_Read(addr + i, g_CommonBuf, BIN_SECTOR_SIZE);
////				
////		if(g_CommonBuf[0] == 0xFF 
////			&& memcmp(g_CommonBuf, &g_CommonBuf[BIN_SECTOR_SIZE], BIN_SECTOR_SIZE) == 0)	//寻找文件结束标志
////		{
////			isFound = True;
////			break;
////		}
////		memcpy(&crcSector, g_CommonBuf, BIN_SECTOR_SIZE);
////	}

////	if(isFound 
////		&& crcSector.m_CodeCrc ==  File_CalcuCrc(addr, crcSector.m_FwBinDataLen)
////		&& i < FILE_BUF_SIZE)
////	{
////		g_Settings.isBleUpg = True;
////		WritePmsFileLen(i);
////		//Printf("pmsLen=%d\n", i);
////	}
////}



////#ifdef CFG_NVC	

////uint8 Pms_GetSocPlayId(uint8 soc)
////{
////	struct
////	{
////		uint8 cmpValue;
////		uint8 playId;
////		uint32 bitMask;
////	}
////	static const playId[] = 
////	{
////		 {100, NVC_BAT_SOC_100, BIT_9}
////		,{90, NVC_BAT_SOC_90, BIT_8}
////		,{80, NVC_BAT_SOC_80, BIT_7}
////		,{70, NVC_BAT_SOC_70, BIT_6}
////		,{60, NVC_BAT_SOC_60, BIT_5}
////		,{50, NVC_BAT_SOC_50, BIT_4}
////		,{40, NVC_BAT_SOC_40, BIT_3}
////		,{30, NVC_BAT_SOC_30, BIT_2}
////		,{20, NVC_BAT_SOC_20, BIT_1}
////		,{10, NVC_BAT_SOC_10, BIT_0}
////	};
////	
////	int i = 0;

////	if(soc == 10) return NVC_BAT_SOC_10;

////	//Get play id
////	for(i = 0; i < GET_ELEMENT_COUNT(playId); i++)
////	{
////		if(playId[i].cmpValue == soc && (g_Settings.SocPlayFlag & playId[i].bitMask))
////		{
////			return playId[i].playId;
////		}
////	}
////	
////	return 0;
////}

////uint8 Pms_GetLpSocPlayId(uint8 soc)
////{
////	int i = 0;
////	//Get play id
////	for(i = 0; i < GET_ELEMENT_COUNT(g_Settings.LoSocPlayFlag); i++)
////	{
////		if(soc == g_Settings.LoSocPlayFlag[i] && g_Settings.LoSocPlayFlag[i])
////		{
////			return NVC_LOW_POWER;
////		}
////	}
////	
////	return 0;
////}

////void Pms_UpdateSoc(const BatStatePkt* pNewPkt)
////{
////	Bool isPlay = False;
////	uint8 soc = Pms_GetAveBatSoc(pNewPkt);
////	uint8 playId = Pms_GetSocPlayId(soc);

////	if(playId && soc != g_pPms->m_playSoc)
////	{
////		isPlay = True;
////		NVC_PLAY(playId);
////	}

////	playId = Pms_GetLpSocPlayId(soc);
////	if(playId && soc != g_pPms->m_playSoc)
////	{
////		isPlay = True;
////		NVC_PLAY(playId);
////	}

////	if(isPlay)
////	{
////		g_pPms->m_playSoc = soc;
////	}
////}
////#endif

//#define BAT_VERIFY_INVALID() (g_Settings.IsBatVerifyEn && (g_pPms->m_Port0Verify ==  BAT_VERIFY_FAILED || g_pPms->m_Port1Verify == BAT_VERIFY_FAILED)) 
//Bool Pms_IsNotDischarge()
//{
//	if(g_pPms->m_isTestBat) 
//	{
//		return False;
//	}
//	else if(g_Settings.IsDeactive)
//	{
//		//对于没被激活的设备，允许放电3分钟，上电试车。
//		if(SwTimer_isTimerOutEx(g_DeactiveTimer, MAX_DEACTIVE_ALLOW_TIME))
//		{
//			g_pPms->m_PowerOffAtOnce = True;
//			return True;
//		}
//		return False;
//	}
//	if(BAT_VERIFY_INVALID())
//	{
//		return True;
//	}
//	
//	return g_Settings.IsAlarmMode || g_Settings.IsForbidDischarge || g_Settings.IsDeactive;
//}

//void Pms_SendCmdDischarge()
//{
//	g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
//}

//void Pms_SetBatVerify(uint8 port0, uint8 port1)
//{
//	Bool isAdd = False;
//	Bool isRemove = False;
//	
//	if(g_pPms->m_portMask & 0x01)
//		g_pPms->m_Port0Verify = port0;
//	if(g_pPms->m_portMask & 0x02)
//		g_pPms->m_Port1Verify = port1;
//	//没有起作用
//	if(g_pPms->m_Port0Verify == BAT_VERIFY_FAILED)
//	{
//		isAdd |= Pms_AddSn(g_pPms->m_batPkt.desc[0].serialNum);
//	}
//	else
//	{
//		isRemove |= Pms_RemoveSn(g_pPms->m_batPkt.desc[0].serialNum);
//	}
//	
//	if(g_pPms->m_Port1Verify == BAT_VERIFY_FAILED)
//	{
//		isAdd |= Pms_AddSn(g_pPms->m_batPkt.desc[1].serialNum);
//	}
//	else
//	{
//		isRemove |= Pms_RemoveSn(g_pPms->m_batPkt.desc[1].serialNum);
//	}
//	//没有起作用
//	if(isAdd || isRemove)
//	{
//		if(isAdd)
//		{
//			LOG2(ET_PMS_BAT_VERIFY, (port1 << 4) | port0, Pms_GetAveBatSoc(Null));
//		}
//		
//		Nvds_Write_SysCfg();
//	}

//	//如果在熄火状态下,立刻触发发送断电命令操作
//	if(BAT_VERIFY_INVALID())
//	{
//		g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
//	}
//	
//	/*
//	PFL(DL_BLE, "BatIsVerify=%d, Port0BatVerfy=%d,Port1BatVerfy=%d\n"
//		, g_Settings.IsBatVerifyEn
//		, g_pPms->m_Port0Verify
//		, g_pPms->m_Port1Verify
//		);
//		*/
//}

////void Pms_UtpRx(uint8 u8InChar)
////{
////	static uint8 buff[UTP_PURE_DATA_SIZE];
////	static int len = 0;
////	#define RX_CHECK_LEN 10
////	static uint8 g_rxLen = 0;
////	g_rxLen = (u8InChar>0) ? 0 : (g_rxLen + 1);
////	//检测每个数据包的前RX_CHECK_LEN个字节，是否为全0，如果是，说明UART有错误，需要重新初始化
////	//修复一个SUART MCU问题
////	if(g_rxLen >= RX_CHECK_LEN && len <= RX_CHECK_LEN)
////	{
////		SwTimer_Start(&g_PmsPwrOffTimer, 1, PMS_TIMEID_START);
////		g_rxLen = 0;
////		return;
////	}
////	
////	if(IsPktEnd(u8InChar, UTP_FRAME_TAIL, buff, &len, sizeof(buff)))
////	{
////		PFL(DL_PMS, "Rx:");
////		DUMP_BYTE_LEVEL(DL_PMS, buff, len);

////		SafeBuf_Write(&g_pUtp->m_SafeBuf, buff, len);
////		len = 0;
////		g_rxLen = 0;
////	}
////}

uint8 Pms_GetBatCount()
{
	return g_pPms->m_batPkt.batteryCount;
}

Bool Pms_IsAccOn()
{
	return (g_pPms->m_deviceState.isAccOn);
}

////void Pms_SwitchProtocol(uint8 protocol)
////{
////	if(g_pPms->m_newProtocol != protocol)
////	{
////		//PFL(DL_PMS, "Switch Protoco to [%s]:", protocol == PROTOCOL_UTP ? "UTP" : "xModem");
////	}
////	g_pPms->m_newProtocol = protocol;
////	
////	if(protocol == PROTOCOL_UTP)
////	{
////		g_pPms->m_Protocol = protocol;
////		g_pPms->m_newProtocol = g_pPms->m_Protocol;
////		Utp_FrameReset(g_pUtp);
////	}
////	else
////	{
////		Utp_FrameReset(g_pUtp);
////		SwTimer_Stop(&g_pPms->m_Timer);	
////	}
////}


////int Pms_BuildSyncStatePkt(uint8* buff)
////{
////	int i = 0;	
////	S_RTC_TIME_DATA_T g_dt = {0};
////	RTC_GetDateAndTime(&g_dt);
////	
////	buff[i++] = (uint8)(g_dt.u32Year - 2000);
////	buff[i++] = (uint8) g_dt.u32Month;
////	buff[i++] = (uint8) g_dt.u32Day;
////	buff[i++] = (uint8) g_dt.u32Hour;
////	buff[i++] = (uint8) g_dt.u32Minute;
////	buff[i++] = (uint8) g_dt.u32Second;
////	
////	buff[i++] = (uint8) g_pSimCard->csq;
////	buff[i++] = (uint8) g_pGps->sateInview;
////	buff[i++] = (uint8) g_pGps->maxSnr;
////	buff[i++] = (uint8) g_pGps->isLocationUpdate;

////	return i;
////}



////void Pms_CheckDevChanged(DevState* pDevState)
////{
////	int remainMin = Sign_GetRemainMin();
////	if(g_pPms->m_deviceState.isAccOn != pDevState->isAccOn)
////	{
////		PFL(DL_MAIN, "ACC %s\n", pDevState->isAccOn ? "ON" : "OFF");
////		g_pPms->m_deviceState.isAccOn = pDevState->isAccOn;
////		PostMsg(pDevState->isAccOn ? MSG_KEY_ON: MSG_KEY_OFF);
////		LOG2(pDevState->isAccOn ? ET_PMS_ACC_ON: ET_PMS_ACC_OFF, (uint8)remainMin, Pms_GetAveBatSoc(Null));
////		Gprs_UpdateFlag(GPRS_EVENT_DEV_CHANGED);

////		if(!pDevState->isAccOn)
////		{
////			if(Pms_IsNotDischarge())
////			{
////				//熄火状态，如果是不允许放电，重新触发一次。
////				g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
////			}
////		}
////		/*
////		if(pDevState->isAccOn )
////		{
////			if(Pms_IsNotDischarge())
////			{
////				Beep_Mode(BEEP_BAT_VERIFY_FAILED);
////			}
////		}
////		*/
////	}
////}

////void Pms_SetPortInfo(const PortDescs* pPortDescs, int len)
////{
////	if(sizeof(PortDescs)+1 == len)	//兼容增加一个字节DevState
////	{
////		uint8* pByte = (uint8*)pPortDescs;
////		Pms_CheckDevChanged((DevState*)pByte);
////		pPortDescs = (PortDescs*)&pByte[1];

////	}
////	else if(sizeof(PortDescs) == len)
////	{
////		
////	}
////	else
////	{
////		//Printf("PortInfo len err\n");
////		return;
////	}
////	
////	//memcpy(&m_BmsVer[0], pPortDescs->BmsVerDesc, sizeof(m_BmsVer));
////}

////void Pms_OnDeviceReset()
////{
////	g_pPms->m_isNotDischarge = False;
////#ifdef CFG_REMOTE_ACC
////	g_pPms->m_pmsAcc = False;
////#endif
////}

////void Pms_OnBatteryFaultChanged(uint8 port, Bool isFault)
////{
////	uint8 flag = Pms_GetPortFault(&g_pPms->m_batPkt.desc[port]);
////	if(isFault)
////	{
////		if(flag)
////		{
////			g_pPms->m_isBatErr |= (1<<port);
////		}
////	}
////	else
////	{
////		g_pPms->m_isBatErr &= ~(1<<port);
////	}
////	
////	LOG2(ET_BMS_FAULT, isFault, flag << 4 | port);
////}

//void Pms_OnAccChanged(Bool isOn)
//{
////	PFL(DL_MAIN, "ACC %s\n", isOn ? "ON" : "OFF");
////	g_pPms->m_deviceState.isAccOn = isOn;
////	PostMsg(isOn ? MSG_KEY_ON: MSG_KEY_OFF);
////	LOG2(isOn ? ET_PMS_ACC_ON: ET_PMS_ACC_OFF, (uint8)Sign_GetRemainMin(), Pms_GetAveBatSoc(Null));
////	Gprs_UpdateFlag(GPRS_EVENT_DEV_CHANGED);

////	if(!isOn)
////	{
////		if(Pms_IsNotDischarge())
////		{
////			//熄火状态，如果是不允许放电，重新触发一次。
////			g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
////		}
////	}
//}

void Pms_OnBatteryPlugIn(uint8 port)
{
//	g_DeactiveBeepFlag = 0;
//	if(g_DeactiveTimer == 0)
//		g_DeactiveTimer = GET_TICKS();
	
	//如果是测试电池，响一声告警音
//	if(g_pPms->m_isTestBat)
//	{
//		NVC_PLAY(NVC_BAT_PLUG_IN);
//	}
//	//插入电池，滴滴提示
//	else// if(g_pPms->m_deviceState.isAccOn)
//	{
//		NVC_PLAY(NVC_INFO);
//	}	
}

void Pms_OnBatteryPlugOut(uint8 port)
{
	//判断是否全部电池拔出
}

void Pms_OnBatteryChanged(uint8 port, Bool isPlugIn)
{
	if(port == 0)
	{
		g_pPms->m_Port0Verify = BAT_VERIFY_NONE;
	}
	else
	{
		g_pPms->m_Port1Verify = BAT_VERIFY_NONE;
	}

//	if(g_pPms->m_portMask)
//		g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
	Printf("%sBattery[%d] %s\n", (g_pPms->m_isTestBat)?"Test ":"", port, isPlugIn ? "in" : "out");
//	LOG2(isPlugIn ? ET_PMS_BAT_PLUG_IN: ET_PMS_BAT_PLUG_OUT, Pms_GetBatCount(), Pms_GetAveBatSoc(Null));
	PostMsg(isPlugIn ? MSG_BATTERY_PLUG_IN :  MSG_BATTERY_PLUG_OUT);
				   
	if(isPlugIn == PLUG_IN)
		Pms_OnBatteryPlugIn(port);
	else
		Pms_OnBatteryPlugOut(port);
}

////void Pms_UpdateDeviceState(const BatStatePkt* pNewPkt, int len, BatStatePkt* pOlderPkt)
////{
////	if(len < BAT_PKT_HEAD) return;
////	
////	//Update device state
////	DevState* pDevState = (DevState*)&pNewPkt->deviceState;
////	#ifdef CFG_REMOTE_ACC
////	//用禁止放电标志取代ACC ON
////	//pDevState->isAccOn = g_Settings.isRemoteAccOn;
////	#endif
////	
////	//("m_isTestBat = %d\n", g_pPms->m_isTestBat);
////	//仅当有电池时，才更新测试电池标志
////	if(pNewPkt->batteryCount)
////	{
////		g_pPms->m_isTestBat = pDevState->isTestBat;
////	}
////	else
////	{
////		//如果没电池，强制ACC值为OFF
////		pDevState->isAccOn = False;
////	}
////	
////	if(g_pPms->m_deviceState.isAccOn != pDevState->isAccOn)
////	{
////		Pms_OnAccChanged(pDevState->isAccOn);
////	}

////	memcpy(&g_pPms->m_deviceState, &pNewPkt->deviceState, 1);

////	


////	if(pOlderPkt->packState != pNewPkt->packState)
////	{
////		LOG2(ET_PACK_STATE_CHANGED, pNewPkt->packState, Pms_GetAveBatSoc(Null));
////	}
////	memcpy(pOlderPkt, pNewPkt, BAT_PKT_HEAD);
////}

////Only for test
//uint8 g_ForceBatSoc = 0;
//void Pms_SetBatInfo(const BatStatePkt* pNewPkt, int len)
//{
////	int remainLen = 0;
////	BatStatePkt pkt = {0};

////	if(pNewPkt)
////	{
////		memcpy(&pkt, pNewPkt, BAT_PKT_HEAD);
////	}
////	else
////	{
////		len = BAT_PKT_HEAD;
////		memset(&pkt, 0, len);
////		pNewPkt = &pkt;
////	}
////	remainLen = len - BAT_PKT_HEAD;

////	//重组成为当前版本的描述符数据包。
////	//判断是否旧版本的长度描述符长度
////	if(remainLen % sizeof(OlderBatteryDesc) == 0)
////	{
////		OlderBatteryDesc* pOlderVerDesc = (OlderBatteryDesc*)pNewPkt->desc;
////		for(int i = 0; i < remainLen / sizeof(OlderBatteryDesc); i++)
////		{
////			memcpy(&pkt.desc[i], pOlderVerDesc++, sizeof(OlderBatteryDesc));
////		}
////	}
////	else
////	{
////		BatteryDesc* pDesc = (BatteryDesc*)pNewPkt->desc;
////		for(int i = 0; i < remainLen / sizeof(BatteryDesc); i++)
////		{
////			memcpy(&pkt.desc[i], pDesc++, sizeof(BatteryDesc));
////		}
////	}
////	
////	//转换电流单位和温度单位
////	pkt.workCurrent = (pkt.workCurrent > 0) ? (pkt.workCurrent - 30000) : 0;
////	for(int i = 0; i < pkt.batteryCount; i++)
////	{
////		BatteryDesc* pDesc = (BatteryDesc*)&pkt.desc[i];
////		
////		pDesc->current = (pDesc->current > 0) ? (pDesc->current - 30000) : 0;
////		pDesc->temp -= 40;
////		pDesc->bmsPcbTemp -= 40;
////		pDesc->connectorTemp -= 40;
////		
////		//Only for test,强制设置电池电量
////		if(g_ForceBatSoc)
////		{
////			pDesc->SOC = g_ForceBatSoc;
////		}
/////*
////		//Only for test,
////		if(g_TestFlag & BIT_0)
////		{
////			BatFault* pBatFault = (BatFault*)&pDesc->fault;
////			pBatFault->OVP = 1;
////			pDesc->maxVoltage = 4250;
////		}
////		if(g_TestFlag & BIT_1)
////		{
////			BatFault* pBatFault = (BatFault*)&pDesc->fault;
////			pBatFault->OTP = 1;
////		}
////		if(g_TestFlag & BIT_2)
////		{
////			DevState* pBatFault = (DevState*)&pkt.deviceState;
////			pBatFault->isLock = 1;
////		}
////		*/
////	}
////	
////#ifdef CFG_NVC	
////	Pms_UpdateSoc(&pkt);
////#endif
////	Pms_UpdateDeviceState(&pkt, BAT_PKT_HEAD, &g_pPms->m_batPkt);
////	Pms_UpdateBatteryInfo(pkt.desc, pkt.batteryCount);
//	
//}

//int Pms_GetBatInfo(uint8 batInd, void* pData)
//{
//	int i = 0;
//	int len = 0;
//	struct _BatInfoPkt
//	{
//		uint8 packInfo[8];
//		uint8 desc[16];
//	}
//	*pBatInfo = (struct _BatInfoPkt*)pData;
//	
//	memcpy(pBatInfo->packInfo, (void*)&g_pPms->m_batPkt.MaxOutPutCurrent, sizeof(pBatInfo->packInfo));
//	len = sizeof(pBatInfo->packInfo);
//	
//	for(i = 0; i < MAX_BATTERY_COUNT; i++)
//	{
//		if((batInd == g_pPms->m_batPkt.desc[i].portId + 1) 
//			&& ((g_pPms->m_portMask >> batInd) & 0x01))
//		{
//			memcpy(&pBatInfo->desc[i], &g_pPms->m_batPkt.desc[i], sizeof(pBatInfo->desc));
//			len += sizeof(struct _BatInfoPkt);
//			break;
//		}
//	}	
//	return len;
//}
////int Pms_GetBatInfo1(uint8 batInd, void* pData)
////{
////	int i = 0;
////	int len = 0;
////	unsigned short vl_pos;
////	st_modbus_reg_unit vl_tmp;
////	unsigned short vl_tmp_value;	
////	struct _GprsBatteryDesc *pBatInfo = pData;	
////	for(i = 0; i < MAX_BATTERY_COUNT; i++)
////	{
////		if((Battery_isIn(i))&&(batInd == i))
////		{
////			vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN34);
////			pBatInfo->serialNum[0] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_H;
////			pBatInfo->serialNum[1] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_L;			
////			vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN56);
////			pBatInfo->serialNum[2] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_H;
////			pBatInfo->serialNum[3] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_L;
////		 
////      		vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN78);
////			pBatInfo->serialNum[4] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_H;
////			pBatInfo->serialNum[5] = gl_bms_info_p[i]->reg_unit[vl_pos].bits16_L;
////			
////			pBatInfo->portId =i;
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SOC)]; // 单位百分比%
////            vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
////			if(0xFFFF == vl_tmp_value)
////			{
////				pBatInfo->SOC = 0xFF;
////			}
////			else
////			{
////				pBatInfo->SOC = vl_tmp_value/10;
////			}
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TVOLT)]; // 10mv
////			pBatInfo->voltage[0] = vl_tmp.bits16_L;
////			pBatInfo->voltage[1] = vl_tmp.bits16_H;
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR)]; // 10mA
////			pBatInfo->current[0] = vl_tmp.bits16_L;
////			pBatInfo->current[1] = vl_tmp.bits16_H;
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HTEMP)]; // 温度，单位℃
////			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
////			if(0xFFFF == vl_tmp_value)
////			{
////				pBatInfo->temp = 0xFF;
////			}
////			else
////			{
////				pBatInfo->temp = vl_tmp_value/10;
////			}
////			
////			/*                
////			Bit      Bit7~ Bit6     Bit5             Bit4    Bit3    Bit2    Bit1    Bit0
////			Fault   Reserved    Other Fault    UTP    OTP    OCP    UVP    OVP
////			*/
////			//这里可能要判断目前是处于充电状态还是放电状态，但是现在先不判断
////			vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT1);
////			vl_tmp = gl_bms_info_p[i]->reg_unit[vl_pos]; // 运行故障字1
////			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
////			pBatInfo->fault = 0;
////			if(vl_tmp_value&(1<<0))
////			{
////				//过压
////				pBatInfo->fault |= (1<<0);
////			}

////			if(vl_tmp_value&((1<<1)|(1<<2)))
////			{
////				//欠压
////				pBatInfo->fault |= (1<<1);
////			}

////			if(vl_tmp_value&((1<<3)|((1<<4)|(1<<5))))
////			{
////				//过流
////				pBatInfo->fault |= (1<<2);
////			}

////			if(vl_tmp_value &((1<<6)|(1<<7)|(1<<10)|(1<<11)|(1<<12)))
////			{
////				//过温
////				pBatInfo->fault |= (1<<3);
////			}

////			if(vl_tmp_value &((1<<8)|(1<<9)))
////			{
////				//低温
////				pBatInfo->fault |= (1<<4);
////			}
////			vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT1);
////			if((vl_tmp_value &((1<<14)|(1<<15)))||
////				(gl_bms_info_p[i]->reg_unit[vl_pos].bits16_L != 0)||
////				(gl_bms_info_p[i]->reg_unit[vl_pos].bits16_H != 0)||
////				(gl_bms_info_p[i]->reg_unit[vl_pos+1].bits16_L != 0)||
////				(gl_bms_info_p[i]->reg_unit[vl_pos+1].bits16_H != 0)||
////				(gl_bms_info_p[i]->reg_unit[vl_pos+3].bits16_L != 0)||
////				(gl_bms_info_p[i]->reg_unit[vl_pos+3].bits16_H != 0))
////			{
////				//其他
////				pBatInfo->fault |= (1<<5);
////			}
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LTSTA)];
////			pBatInfo->damage = ((vl_tmp.bits16_L<<1)&0x02)|((vl_tmp.bits16_L>>1)&0x01);

////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CYCLE)];
////			pBatInfo->cycleCount[0] = vl_tmp.bits16_L;
////			pBatInfo->cycleCount[1] = vl_tmp.bits16_H;
////			
////			
////			len += 17;//sizeof(GprsBatteryDesc);
////			break;
////		}
////	}	
////	return len;
////}
////int Pms_GetBatInfo2(uint8 batInd, void* pData)
////{
////	int i = 0;
////	int len = 0;
//////	unsigned short vl_pos;
////	st_modbus_reg_unit vl_tmp;
////	unsigned short vl_tmp_value;	
////	struct _GprsBatteryDesc *pBatInfo = pData;	
////	for(i = 0; i < MAX_BATTERY_COUNT; i++)
////	{
////		if((Battery_isIn(i))&&(batInd == i))
////		{	
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HVOLT)]; 
////			pBatInfo->maxVoltage[0] = vl_tmp.bits16_L; //mV
////			pBatInfo->maxVoltage[1] = vl_tmp.bits16_H; //mV

////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LVOLT)]; 
////			pBatInfo->minVoltage[0] = vl_tmp.bits16_L; //mV
////			pBatInfo->minVoltage[1] = vl_tmp.bits16_H; //mV

////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HVNUM)]; 
////			pBatInfo->maxVolCell = vl_tmp.bits16_L;
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LVNUM)]; 
////			pBatInfo->minVolCell = vl_tmp.bits16_L;

////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DMOST)]; 
////			vl_tmp_value = (vl_tmp.bits16_L + (vl_tmp.bits16_H << 8));
////			if(0xFFFF == vl_tmp_value)
////			{
////				pBatInfo->bmsPcbTemp = 0xFF;
////			}
////			else
////			{
////				pBatInfo->bmsPcbTemp = vl_tmp_value/10;
////			}
////			
////			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CONT)]; 
////			vl_tmp_value = (vl_tmp.bits16_L + (vl_tmp.bits16_H << 8));
////			if(0xFFFF == vl_tmp_value)
////			{
////				pBatInfo->connectorTemp = 0xFF;
////			}
////			else
////			{
////				pBatInfo->connectorTemp = vl_tmp_value/10;
////			}

////			vl_tmp_value = Battery_get_switch_state(i);
////			pBatInfo->mosState = (vl_tmp_value & 0x0003);
////			
////			len += 9;//sizeof(GprsBatteryDesc);
////			break;
////		}
////	}	
////	return len;
////}
//int Pms_GetBatInfo3(uint8 batInd, void* pData)
//{
//	int i = 0;
//	int len = 0;
////	unsigned short vl_pos;
//	st_modbus_reg_unit vl_tmp;
////	unsigned short vl_tmp_value;	
//	struct _GprsBatteryDesc *pBatInfo = pData;	
//	for(i = 0; i < MAX_BATTERY_COUNT; i++)
//	{
//		if((Battery_isIn(i))&&(batInd == i))
//		{	
//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_PRVER)]; 
//			pBatInfo->version = vl_tmp.bits16_L; //协议版本号

//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HWVER)]; 
//			pBatInfo->hwver[0] = vl_tmp.bits16_L; //硬件版本号
//			pBatInfo->hwver[1] = vl_tmp.bits16_H;
//			
//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_BLVER)]; 
//			pBatInfo->blver[0] = vl_tmp.bits16_L; //Bootloader版本
//			pBatInfo->blver[1] = vl_tmp.bits16_H;

//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_FWMSV)]; 
//			pBatInfo->hwver[0] = vl_tmp.bits16_L; //固件版本号
//			pBatInfo->hwver[1] = vl_tmp.bits16_H;
//			
//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_FWREV)]; 
//			pBatInfo->fwminorver = vl_tmp.bits16_L;//固件版本- 修正版本

//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_FWBNH)]; 
//			pBatInfo->fwbuidnum[0] = vl_tmp.bits16_L; //固件版本-编译版本
//			pBatInfo->fwbuidnum[1] = vl_tmp.bits16_H;
//			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_FWBNL)]; 
//			pBatInfo->fwbuidnum[2] = vl_tmp.bits16_L;
//			pBatInfo->fwbuidnum[3] = vl_tmp.bits16_H;
//			
//			len += 13;//sizeof(GprsBatteryDesc);
//			break;
//		}
//	}	
//	return len;
//}
//int Pms_GetPortInfo(PortStateChangedPkt* portPkt)
//{
//	int i = 0;
////	int j = 0;
//	
//	BatStatePkt* pBatPkt = &g_pPms->m_batPkt;
//	PortDesc* pPortDesc = portPkt->portDesc;

//	memset(portPkt, 0, sizeof(PortStateChangedPkt));

//	portPkt->portMask 		= g_pPms->m_portMask;

//	for(i = 0; i < MAX_BATTERY_COUNT; i++)
//	{
//		if(pBatPkt->desc[i].capacity)
//		{
//			pPortDesc = &portPkt->portDesc[i];
//			
//			pPortDesc->portId 		= pBatPkt->desc[i].portId;
//			memcpy(pPortDesc->serialNum, pBatPkt->desc[i].serialNum, SERIAL_NUM_SIZE);
//			pPortDesc->rateVoltage 	= pBatPkt->desc[i].rateVoltage;
//			pPortDesc->rateCurrent 	= pBatPkt->desc[i].rateCurrent;
//			pPortDesc->capacity 	= pBatPkt->desc[i].capacity;
//		}
//	}
//	
//	return sizeof(PortStateChangedPkt);
//}

////OP_CODE Pms_EventSwitchProtocol(uint8 cmd, const uint8* pData, uint8 dataLen, uint8* rspData, uint8* pRspLen)
////{
////	Pms_SwitchProtocol(pData[0]);
////	
////	return OP_SUCCESS;
////}

////OP_CODE Pms_EventLog(uint8 cmd, const uint8* pData, uint8 dataLen, uint8* rspData, uint8* pRspLen)
////{
////	LOG2(ET_PMS_LOG, pData[0], pData[1]);
////	
////	return OP_SUCCESS;
////}

////OP_CODE Pms_BmsFaultChanged(uint8 cmd, const uint8* pData, uint8 dataLen, uint8* rspData, uint8* pRspLen)
////{
////	BmsFault* pBmsFault = (BmsFault*)pData;
////	uint8	 flag = 0;
////	Bool isChanged = False;
////	if(dataLen < sizeof(BmsFault) * 2)
////	{
////		//PFL(DL_PMS, "Param len error, %d", sizeof(BmsFault));
////		return OP_PARAM_INVALID;
////	}

////	if(Battery_isErr(&g_pPms->m_BmsFault[0]) != Battery_isErr(&pBmsFault[0]))
////	{
////		isChanged = True;
////	}
////	else if(Battery_isErr(&g_pPms->m_BmsFault[1]) != Battery_isErr(&pBmsFault[1]))
////	{
////		isChanged = True;
////	}
////	
////	if(Battery_isErr(&pBmsFault[0]))
////		flag |= BIT_0;
////	if(Battery_isErr(&pBmsFault[1]))
////		flag |= BIT_1;
////	
////	g_pPms->m_isBmsFault = flag;
////	
////	memcpy(g_pPms->m_BmsFault, pData, sizeof(g_pPms->m_BmsFault));

////	if(isChanged)
////		LOG2(ET_BMS_FAULT, g_pPms->m_isBmsFault, flag);
////	
////	return OP_SUCCESS;
////}

//#ifdef CFG_CABIN_LOCK
//void Pms_SetCabinLock(Bool isLock)
//{
//	if(isLock)
//	{
//		Beep_Mode(BEEP_LOCK);
//	}
//	else
//	{
//		Beep_Mode(BEEP_UNLOCK);
//	}

//	Cabin_UnLock();
//	LOG2(ET_CABIN_LOCK, 0, isLock);
//}
//#endif //CFG_CABIN_LOCK



//#ifdef CFG_REMOTE_ACC
//void Pms_SetAcc(Bool isOn)
//{
//	if(g_Settings.IsForbidDischarge)
//	{
//		//isOn = False;
//		if(g_pPms->m_deviceState.isAccOn)
//		{
//			//Force to False
//			Pms_OnAccChanged(False);
//		}
//	}
//	
//	g_Settings.isRemoteAccOn = isOn;
//	g_pPms->m_pmsAcc = !isOn;
//	
//	PostMsg(isOn ? MSG_KEY_ON: MSG_KEY_OFF);
//	LOG2(isOn ? ET_PMS_ACC_ON: ET_PMS_ACC_OFF, (uint8)Sign_GetRemainMin(), Pms_GetAveBatSoc(Null));
//	
//	if(isOn)
//	{
//		Beep_Mode(BEEP_LOCK);
//	}
//	else
//	{
//		Beep_Mode(BEEP_UNLOCK);

//		//启动定时器，延时1秒再发送Acc Off到Pms,保证提示音能够发出完整声音
//		//如果不延时，可能会导致提示音发不出来,被Acc Off关掉。
//		g_pPms->m_pmsAcc = isOn;
////		SwTimer_Start(&g_PmsAccOffTimer, 1000, PMS_TIMEID_ACCOFF);
//		if(Pms_IsNotDischarge())
//		{
//			//熄火状态，如果是不允许放电，重新触发一次。
//			g_pPms->m_isNotDischarge = !Pms_IsNotDischarge();
//		}
//	}
//	
//}
//#endif //CFG_REMOTE_ACC



////void Pms_UpgradePms()
////{
////}

////void Pms_Reset()
////{
////}
//Bool Pms_FwIsValid()
//{
//	return False;
//}
////Bool Pms_SendCtrlCmd()
////{
////}
//void Pms_Beep()
//{
//	static Bool isSwitchToOn = True;
//	static uint32 alarmInitTicks = 0;
//		
//#ifdef CFG_REMOTE_ACC
//	if(Pms_IsAccOn() || g_Settings.isRemoteAccOn)
//#else
//	if(Pms_IsAccOn())
//#endif
//	{
//		if(Pms_IsNotDischarge() && IS_BAT_ON() && !g_pPms->m_isTestBat)
//		{
//			if(g_Settings.IsDeactive)
//			{
//				//没激活时,仅响一次
//				if(isSwitchToOn)
//				{
//					//Beep_Mode(BEEP_DEACTIVE);
//				}
//			}
//			else
//			{
//				//周期告警
//				if(SwTimer_isTimerOutEx(alarmInitTicks, 6000))
//				{
//					//Printf("BEEP_ILLEG_MOVING\n");
//					alarmInitTicks = GET_TICKS();
//					Beep_Mode(BEEP_ILLEG_MOVING);
//				}
//			}
//		}
//		else if(g_pPms->m_isBatErr)
//		{
//			if(SwTimer_isTimerOutEx(alarmInitTicks, 3000))
//			{
//				alarmInitTicks = GET_TICKS();
//				Beep_Mode(BEEP_BAT_FAULT);
//			}
//		}

//		
//		isSwitchToOn = False;
//	}
//	else
//	{
//		isSwitchToOn = True;
//		alarmInitTicks = 0;
//	}
//	if(g_pPms->m_batPkt.packState == PACK_STATE_CHARGE)
//	{	//如果在充电，则发出告警声
//		if(SwTimer_isTimerOutEx(alarmInitTicks, 2000))
//		{
//			alarmInitTicks = GET_TICKS();
//			Beep_Mode(BEEP_BAT_FAULT);
//		}
//	}

//	if(g_Settings.IsDeactive && IS_BAT_ON() && !g_pPms->m_isTestBat && g_DeactiveTimer)
//	{
//		if(g_DeactiveBeepFlag == 0 && SwTimer_isTimerOutEx(g_DeactiveTimer, 1))
//		{
//			g_DeactiveBeepFlag = 1;
//			Beep_Mode(BEEP_DEACTIVE);
//		}
//		if(g_DeactiveBeepFlag == 1 && SwTimer_isTimerOutEx(g_DeactiveTimer, MAX_DEACTIVE_ALLOW_TIME-30000))
//		{
//			//到数30秒响两下告警
//			g_DeactiveBeepFlag = 2;
//			Beep_Mode(BEEP_BAT_FAULT);
//		}
//		if(g_DeactiveBeepFlag == 2 && SwTimer_isTimerOutEx(g_DeactiveTimer, MAX_DEACTIVE_ALLOW_TIME-5000))
//		{
//			//到数5秒响一下告警
//			g_DeactiveBeepFlag = 3;
//			Beep_Mode(BEEP_ALARM_MODE_EN);
//		}
//	}
//	
//}

/*!
 * \brief 查询参数变化，主动上报信息
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
int slave_rs485_cmd_param_changed_polling(void)
{
    unsigned char i;
    static unsigned char sl_init_flag = 1;
    if(sl_init_flag)
    {
        sl_init_flag = 0;
        for(i = 0; i < NFC_READER_COUNT_MAX; i++)
        {
            gl_base_param[i].soc.bits16_H = 0;
            gl_base_param[i].soc.bits16_L = 0;
            gl_base_param[i].fault[0].bits16_H = 0;
            gl_base_param[i].fault[0].bits16_L = 0;
            gl_base_param[i].fault[1].bits16_H = 0;
            gl_base_param[i].fault[1].bits16_L = 0;
            gl_base_param[i].fault[2].bits16_H = 0;
            gl_base_param[i].fault[2].bits16_L = 0;
            gl_base_param[i].fault[3].bits16_H = 0;
            gl_base_param[i].fault[3].bits16_L = 0;
            gl_base_param[i].online = 0;
            gl_base_param[i].online_delay_cnt = 0;
        }
    }
//    在读取到电池信息后，才进入下面程序，判断参数变化	
	for(i = 0; i < NFC_READER_COUNT_MAX; i++)
	{
        //比较在线
		if(TRUE == slave_rs485_is_bat_valid(i))
		{
			//在线
			if((!gl_base_param[i].online))
			{
				//上报
				gl_base_param[i].online = 1;
//				pmsPortEventPost(EV_PMS_ONLINE);					
				Pms_OnBatteryChanged(i, PLUG_IN);
			}
		}
		else
		{
			//离线
			if((gl_base_param[i].online))
			{
				//上报
				gl_base_param[i].online = 0;
//				pmsPortEventPost(EV_PMS_ONLINE);
				Pms_OnBatteryChanged(i, PLUG_OUT);		
			}
		}
            
    }
    return 0;
}
//unsigned short Battery_get_FWMSV_version(unsigned char bms_index)
//{
//    //充放电都打开时候才算打开
//    unsigned short vl_pos, vl_HW_version;
//    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_FWMSV);
//    vl_HW_version = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
//        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

//    return vl_HW_version;
//}
////判断是否需要设置或者清除测试模式
//void Battery_check_and_process_testing_mode(void)
//{
//	uint8 i;
//	uint8 testbat_flag =0;
//	for(i = 0; i < MAX_BATTERY_COUNT; i++)
//	{
//		if(slave_rs485_is_bat_valid(i))
//		{
//			if(0xFEFE == Battery_get_FWMSV_version(i))
//				testbat_flag =1;
//		}
//	}
//	if(testbat_flag == 1)
//	{
////		if(!(g_pPms->m_isTestBat))
////		{
//			g_pPms->m_isTestBat = 1;
////		}	
//	}
//	else
//	{
//		g_pPms->m_isTestBat = 0;
//	}

//}
////判断电池充放电管状态
////两个电池时，只要有充放电管打开就认为是打开
//void Battery_check_packstate(void)
//{
//	uint8 i;
//	uint8 packstate =0;
//	for(i = 0; i < MAX_BATTERY_COUNT; i++)
//	{
//		if(slave_rs485_is_bat_valid(i))
//		{
//			if(Battery_get_switch_state(i)&0x01)
//				packstate |= 0x01;
//			if(Battery_get_switch_state(i)&0x02)
//				packstate |= 0x02;
//		}
//	}
//	g_pPms->m_batPkt.packState = packstate;
//}

/*!
 * \brief 电池信息更新
 *		  
 * \param gl_bms_info_p 电池NFC数组
 *
 * \return none
 */
void Pms_Run()
{
	uint8 i;
	uint8 bat_num = 0;
	BatteryDesc* pDesc;
	st_modbus_reg_unit vl_tmp;
    unsigned short vl_tmp_value;
	uint8_t dat;
	//更新电池信息
	for(i = 0; i < NFC_READER_COUNT_MAX; i++)
	{
		pDesc = &g_pPms->m_batPkt.desc[i];
		if(slave_rs485_is_bat_valid(i))
		{
			bat_num ++;
			g_pPms->m_portMask |= (1 << i) ;			 
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN34)];
			pDesc->serialNum[0] = vl_tmp.bits16_H;
			pDesc->serialNum[1] = vl_tmp.bits16_L;
					
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN56)];
			pDesc->serialNum[2] = vl_tmp.bits16_H;
			pDesc->serialNum[3] = vl_tmp.bits16_L;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SN78)];
			pDesc->serialNum[4] = vl_tmp.bits16_H;
			pDesc->serialNum[5] = vl_tmp.bits16_L;
			
			pDesc->portId = i;
						
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SOC)]; // 单位百分比%
            vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);			
			if(0xFFFF == vl_tmp_value)
				dat = 0xFF;
			else
				dat = vl_tmp_value/10;
			pDesc->SOC = dat;
			
			pDesc->voltage = Battery_get_voltage(i);
			pDesc->current = Battery_get_current(i);
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HTEMP)]; //温度，单位℃ 
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			if(0xFFFF == vl_tmp_value)
			{
				dat = 0xFF;
			}
			else
			{
				dat = vl_tmp_value/10;
			}
			pDesc->temp = dat;			
			/*                
			Bit      Bit7~ Bit6     Bit5             Bit4    Bit3    Bit2    Bit1    Bit0
			Fault   Reserved    Other Fault    UTP    OTP    OCP    UVP    OVP
			*/
			//这里可能要判断目前是处于充电状态还是放电状态，但是现在先不判断
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT1)]; //运行故障字1
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->fault = 0;
			if(vl_tmp_value&(1<<0))//过压
				pDesc->fault |= (1<<0);
			if(vl_tmp_value&((1<<1)|(1<<2)))//欠压
				pDesc->fault |= (1<<1);
			if(vl_tmp_value&((1<<3)|((1<<4)|(1<<5))))//过流
				pDesc->fault |= (1<<2);
			if(vl_tmp_value &((1<<6)|(1<<7)|(1<<10)|(1<<11)|(1<<12)))//过温
				pDesc->fault |= (1<<3);
			if(vl_tmp_value &((1<<8)|(1<<9)))//低温
				pDesc->fault |= (1<<4);
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT1)];
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			if((vl_tmp_value &((1<<14)|(1<<15)))||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT2)].bits16_L != 0)||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT2)].bits16_H != 0)||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT2)].bits16_L != 0)||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT2)].bits16_H != 0)||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPWARN1)].bits16_L != 0)||
				(gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPWARN1)].bits16_H != 0))
			{
				//其他
				pDesc->fault |= (1<<5);
			}
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LTSTA)]; 
			pDesc->damage = ((vl_tmp.bits16_L<<1)&0x02)|((vl_tmp.bits16_L>>1)&0x01);

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CYCLE)];
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->cycleCount = vl_tmp_value;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_BVOLT)];//额定电压
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->rateVoltage = vl_tmp_value;
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DSOP)];//额定电流
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->rateCurrent = vl_tmp_value;
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_BCAP)];//
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->capacity = vl_tmp_value;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HVOLT)];//
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->maxVoltage = vl_tmp_value;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LVOLT)];//
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->minVoltage = vl_tmp_value;	

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_HVNUM)];//
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->maxVolCell = vl_tmp_value;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_LVNUM)];//
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			pDesc->minVolCell = vl_tmp_value;

			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DMOST)]; //MOS温度，单位℃ 
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			if(0xFFFF == vl_tmp_value)
			{
				dat = 0xFF;
			}
			else
			{
				dat = vl_tmp_value/10;
			}
			pDesc->bmsPcbTemp = dat;	
			
			vl_tmp = gl_bms_info_p[i]->reg_unit[MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CONT)]; //连接器温度，单位℃ 
			vl_tmp_value = ((vl_tmp.bits16_H<<8)+vl_tmp.bits16_L);
			if(0xFFFF == vl_tmp_value)
			{
				dat = 0xFF;
			}
			else
			{
				dat = vl_tmp_value/10;
			}
			pDesc->connectorTemp = dat;
			
			pDesc->mosState = (Battery_get_switch_state(i) & 0x0003);
		}
		else//置零
		{
			memset(pDesc,0,sizeof(BatteryDesc));
		}
		//!!需要完善
//		if(pTempDesc->fault != pNewDesc->fault)
//		{
//			Pms_OnBatteryFaultChanged(i, pNewDesc->fault);
//		}
	}
	g_pPms->m_totalPort = bat_num;
	g_pPms->m_deviceState.isAccOn = g_Settings.isRemoteAccOn;
	g_pPms->m_deviceState.isTestBat = g_pPms->m_isTestBat;
	g_pPms->m_batPkt.deviceState = g_pPms->m_deviceState.isAccOn;
	g_pPms->m_batPkt.MaxOutPutCurrent = pms_get_max_output_current();
	g_pPms->m_batPkt.workVoltate = pms_get_work_voltate();
	g_pPms->m_batPkt.workCurrent = pms_get_work_current();
	g_pPms->m_batPkt.batteryCount = bat_num;
	g_pPms->m_batPkt.size = sizeof(BatStatePkt);
//	Battery_check_and_process_testing_mode();
//	Battery_check_packstate();
	//喇叭响报警
//	Pms_Beep();
	//拆开检测存储
//	if(state_take_part && (!g_Settings.isTakeApart))
//	{
//		g_Settings.isTakeApart =1;
//		LOG2(ET_TAKE_APART, g_Settings.devcfg, 0);
//	}
}

void Pms_Init()
{
	memset(g_pPms, 0, sizeof(Pms));
}
