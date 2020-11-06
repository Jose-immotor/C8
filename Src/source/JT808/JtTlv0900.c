
#include "Common.h"
#include "JtTlv0900.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"
#include "Battery.h"


static TlvOutMgr g_jtTlvMgr_0900;


static uint8	g_smartState = 0x01;
static uint8	g_smartState_mirror;


static TlvPMSState	g_tlvPMSState[MAX_BAT_COUNT] = {0x00};	// PMS运行参数
static TlvPMSState g_tlvPMSState_mirror[MAX_BAT_COUNT];

static TlvBatInfo g_tlvBatInfo[MAX_BAT_COUNT];	// 电池基本信息
static TlvBatInfo g_tlvBatInfo_mirror[MAX_BAT_COUNT];

static TlvBatWorkInfo g_tlvBatWorkInfo[MAX_BAT_COUNT];	// 电池工作参数
static TlvBatWorkInfo g_tlvBatWorkInfo_mirror[MAX_BAT_COUNT];

static TlvBatTemp g_tlvBatTemp[MAX_BAT_COUNT];	// 电池温度数据
static TlvBatTemp g_tlvBatTemp_mirror[MAX_BAT_COUNT];

static TlvBatFault g_tlvBatFault[MAX_BAT_COUNT];	// 电池故障数据
static TlvBatFault g_tlvBatFault_mirror[MAX_BAT_COUNT];

extern Battery g_Bat[MAX_BAT_COUNT];	

/*
void JtTlv0900_updateBatInfo(uint8 port)
{
	TlvBatInfo* pBat = &g_tlvBatInfo[port];
	pBat->port = port;
	memcpy(g_tlvBatInfo->bid, Bat_getBID(&g_Bat[port]), BMS_ID_SIZE);
	pBat->nominalCur = 0;	//??
	pBat->nominalVol = 0;	//??
}
*/

void JtTlv0900_updateMirror(const uint8* data, int len)
{
	TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, data, len);
}

/*
*/
static void _getCurSmart(void){}
static void _getCurPMSState(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		g_tlvPMSState[i].pmsTemp = SWAP16( g_Bat[i].bmsInfo.htemp );	// PMS温度
		g_tlvPMSState[i].batState = g_Bat[i].presentStatus == BAT_IN ? 0x01 : 0x00 ; // 电池在位状态
		
	}
}

static void _getCurBatInfo(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		g_tlvBatInfo[i].bid[0] = g_Bat[i].bmsInfo.userId[1] >> 8;	// BID
		g_tlvBatInfo[i].bid[1] = g_Bat[i].bmsInfo.userId[1] & 0xFF;
		g_tlvBatInfo[i].bid[2] = g_Bat[i].bmsInfo.userId[2] >> 8;
		g_tlvBatInfo[i].bid[3] = g_Bat[i].bmsInfo.userId[2] & 0xFF;
		g_tlvBatInfo[i].bid[4] = g_Bat[i].bmsInfo.userId[3] >> 8;
		g_tlvBatInfo[i].bid[5] = g_Bat[i].bmsInfo.userId[3] & 0xFF;
		//
		g_tlvBatInfo[i].port = i ;	//								// 电池槽位号
		g_tlvBatInfo[i].nominalVol = SWAP16( g_Bat[i].bmsID.bvolt ) ;	// 额定电压
		g_tlvBatInfo[i].nominalCur = SWAP16( g_Bat[i].bmsID.bcap ) ;	// 额定电流
		
		
	}
}

static void _getCurBatWork(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		g_tlvBatWorkInfo[i].bid[0] = g_Bat[i].bmsInfo.userId[1] >> 8;	// BID
		g_tlvBatWorkInfo[i].bid[1] = g_Bat[i].bmsInfo.userId[1] & 0xFF;
		g_tlvBatWorkInfo[i].bid[2] = g_Bat[i].bmsInfo.userId[2] >> 8;
		g_tlvBatWorkInfo[i].bid[3] = g_Bat[i].bmsInfo.userId[2] & 0xFF;
		g_tlvBatWorkInfo[i].bid[4] = g_Bat[i].bmsInfo.userId[3] >> 8;
		g_tlvBatWorkInfo[i].bid[5] = g_Bat[i].bmsInfo.userId[3] & 0xFF;
		//
		g_tlvBatWorkInfo[i].soc = g_Bat[i].bmsInfo.soc;					// SOC
		g_tlvBatWorkInfo[i].voltage = SWAP16( g_Bat[i].bmsInfo.tvolt );	// 电压
		g_tlvBatWorkInfo[i].current = SWAP16( g_Bat[i].bmsInfo.tcurr );	// 电流
		g_tlvBatWorkInfo[i].maxCellVol = SWAP16( g_Bat[i].bmsInfo.hvolt );
		g_tlvBatWorkInfo[i].minCellVol = SWAP16( g_Bat[i].bmsInfo.lvolt);
		g_tlvBatWorkInfo[i].maxCellNum = SWAP16( g_Bat[i].bmsInfo.hvnum);
		g_tlvBatWorkInfo[i].minCellNum = SWAP16( g_Bat[i].bmsInfo.lvnum);

		// bsminfo中暂时没有
		g_tlvBatWorkInfo[i].maxChgCurr = 0x00;//SWAP16( g_Bat[i].bmsInfo.csop);
		g_tlvBatWorkInfo[i].maxDischgCurr = 0x00;//SWAP16( g_Bat[i].bmsInfo.dsop) ;
		g_tlvBatWorkInfo[i].curWorkFeature = 0x00;//SWAP16( g_Bat[i].bmsInfo.fcap) ;
		//
		g_tlvBatWorkInfo[i].cycCount = SWAP16( g_Bat[i].bmsInfo.cycle) ;
	}
}

static void _getCurBatTemp(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		g_tlvBatTemp[i].bid[0] = g_Bat[i].bmsInfo.userId[1] >> 8;
		g_tlvBatTemp[i].bid[1] = g_Bat[i].bmsInfo.userId[1] & 0xFF;
		g_tlvBatTemp[i].bid[2] = g_Bat[i].bmsInfo.userId[2] >> 8;
		g_tlvBatTemp[i].bid[3] = g_Bat[i].bmsInfo.userId[2] & 0xFF;
		g_tlvBatTemp[i].bid[4] = g_Bat[i].bmsInfo.userId[3] >> 8;
		g_tlvBatTemp[i].bid[5] = g_Bat[i].bmsInfo.userId[3] & 0xFF;
		//
		g_tlvBatTemp[i].cMostTemp = SWAP16( g_Bat[i].bmsInfo.cmost );	// 充电MOS温度
		g_tlvBatTemp[i].dMostTemp = SWAP16( g_Bat[i].bmsInfo.dmost );	// 放电MOS温度
		g_tlvBatTemp[i].fuelTemp = SWAP16( g_Bat[i].bmsInfo.pret);	// 电量计温度
		g_tlvBatTemp[i].contTemp = SWAP16( g_Bat[i].bmsInfo.cont ) ;	// 连接器温度
		g_tlvBatTemp[i].batTemp1 = SWAP16( g_Bat[i].bmsInfo.btemp[0] );	// 电池1温度
		g_tlvBatTemp[i].batTemp2 = SWAP16( g_Bat[i].bmsInfo.btemp[1] );	// 电池2温度
		// bmsinof 暂时没有,待添加
		g_tlvBatTemp[i].tvsTemp = 0 ;	// tvs 温度	
		g_tlvBatTemp[i].fuseTemp = 0 ;	// 保险丝温度
	}
}

static void _getCurBatFault(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		g_tlvBatFault[i].bid[0] = g_Bat[i].bmsInfo.userId[1] >> 8;
		g_tlvBatFault[i].bid[1] = g_Bat[i].bmsInfo.userId[1] & 0xFF;
		g_tlvBatFault[i].bid[2] = g_Bat[i].bmsInfo.userId[2] >> 8;
		g_tlvBatFault[i].bid[3] = g_Bat[i].bmsInfo.userId[2] & 0xFF;
		g_tlvBatFault[i].bid[4] = g_Bat[i].bmsInfo.userId[3] >> 8;
		g_tlvBatFault[i].bid[5] = g_Bat[i].bmsInfo.userId[3] & 0xFF;
		//
		g_tlvBatFault[i].devft1 = SWAP16( g_Bat[i].bmsInfo.devft1);	// 故障1
		g_tlvBatFault[i].devft2 = SWAP16( g_Bat[i].bmsInfo.devft2);
		g_tlvBatFault[i].opft1 = SWAP16( g_Bat[i].bmsInfo.opft1);
		g_tlvBatFault[i].opft2 = SWAP16( g_Bat[i].bmsInfo.opft2);
		g_tlvBatFault[i].opwarn1 = SWAP16( g_Bat[i].bmsInfo.opwarn1);
		g_tlvBatFault[i].opwarn2 = SWAP16( g_Bat[i].bmsInfo.opwarn2);
	}
}


void JtTlv0900_updateStorage( void )		// 
{
	_getCurSmart();
	_getCurPMSState();
	_getCurBatInfo();
	_getCurBatWork();
	_getCurBatTemp();
	_getCurBatFault();
}


int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvMgr_0900, buf, len, tlvCount);
}

void JtTlv0900_init()
{
#define TLV_OUT_COUNT	6
	static TlvOutEx g_tlvBuf_0900Ex[TLV_OUT_COUNT];
	const static TlvOut g_tlv_0900[TLV_OUT_COUNT] =
	{
		{"SMART", &g_tlvBuf_0900Ex[0], TAG_SMART_STATE ,  sizeof(g_smartState), &g_smartState, DT_UINT8, &g_smartState_mirror, Null, 0, 0},
		{"PMS_STATE", &g_tlvBuf_0900Ex[1], TAG_PMS_STATE , MAX_BAT_COUNT * sizeof(TlvPMSState), &g_tlvPMSState[0], DT_STRUCT, &g_tlvPMSState_mirror[0], Null, 0, 0},
		//
		{"BAT_INFO", &g_tlvBuf_0900Ex[2], TAG_BAT_INFO		  , MAX_BAT_COUNT * sizeof(TlvBatInfo), &g_tlvBatInfo[0]    , DT_STRUCT, &g_tlvBatInfo_mirror[0] , Null, 0, 0},
		{"BAT_WORK", &g_tlvBuf_0900Ex[3], TAG_BAT_WORK_PARAM ,MAX_BAT_COUNT * sizeof(TlvBatWorkInfo), &g_tlvBatWorkInfo[0], DT_STRUCT, &g_tlvBatWorkInfo_mirror[0], Null, 0, 0},
		//
		{"BAT_TEMP"	, &g_tlvBuf_0900Ex[4], TAG_BAT_TEMP ,MAX_BAT_COUNT * sizeof(TlvBatTemp), &g_tlvBatTemp[0], DT_STRUCT, &g_tlvBatTemp_mirror[0], Null, 0, 0},
		{"BAT_FAULT", &g_tlvBuf_0900Ex[5], TAG_BAT_FAULT , MAX_BAT_COUNT * sizeof(TlvBatFault), &g_tlvBatFault[0], DT_STRUCT, &g_tlvBatFault_mirror[0], Null, 0, 0},
	};

	TlvOutMgr_init(&g_jtTlvMgr_0900, g_tlv_0900, GET_ELEMENT_COUNT(g_tlv_0900), 1, True);
}
