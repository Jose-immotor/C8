
#include "Common.h"
#include "JtTlv0900.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"

static TlvOutMgr g_jtTlvMgr_0900;


static uint8	g_smartState = 0x01;
static uint8	g_smartState_mirror;


static TlvPMSState	g_tlvPMSState[MAX_BAT_COUNT] = {0x01,0x02};	// PMS运行参数
static TlvPMSState g_tlvPMSState_mirror[MAX_BAT_COUNT];

static TlvBatInfo g_tlvBatInfo[MAX_BAT_COUNT];	// 电池基本信息
static TlvBatInfo g_tlvBatInfo_mirror[MAX_BAT_COUNT];

static TlvBatWorkInfo g_tlvBatWorkInfo[MAX_BAT_COUNT];	// 电池工作参数
static TlvBatWorkInfo g_tlvBatWorkInfo_mirror[MAX_BAT_COUNT];

static TlvBatTemp g_tlvBatTemp[MAX_BAT_COUNT];	// 电池温度数据
static TlvBatTemp g_tlvBatTemp_mirror[MAX_BAT_COUNT];

static TlvBatFault g_tlvBatFault[MAX_BAT_COUNT];	// 电池故障数据
static TlvBatFault g_tlvBatFault_mirror[MAX_BAT_COUNT];

	

void JtTlv0900_updateBatInfo(uint8 port)
{
	TlvBatInfo* pBat = &g_tlvBatInfo[port];
	pBat->port = port;
	memcpy(g_tlvBatInfo->bid, Bat_getBID(&g_Bat[port]), BMS_ID_SIZE);
	pBat->nominalCur = 0;	//??
	pBat->nominalVol = 0;	//??
}

void JtTlv0900_updateMirror(const uint8* data, int len)
{
	TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, data, len);
}

void JtTlv0900_updateStorage(const uint8* data, int len)		// 
{
	TlvInMgr_updateStorage(&g_jtTlvMgr_0900, data, len );
}


int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvMgr_0900, buf, len, tlvCount);
}

void JtTlv0900_init()
{
#define TLV_OUT_COUNT	6
	static TlvOutEx g_tlvBuf_0900Ex[TLV_OUT_COUNT];
	static const TlvOut g_tlv_0900[TLV_OUT_COUNT] =
	{
		{"SMART", &g_tlvBuf_0900Ex[0], TAG_SMART_STATE ,  sizeof(g_smartState), &g_smartState, DT_UINT8, &g_smartState_mirror, Null, 0, 0},
		{"PMS_STATE", &g_tlvBuf_0900Ex[1], TAG_PMS_STATE , MAX_BAT_COUNT * sizeof(TlvPMSState), &g_tlvPMSState[0], DT_STRUCT, &g_tlvPMSState_mirror[0], Null, 0, 0},
		//
		{"BAT_INFO", &g_tlvBuf_0900Ex[2], TAG_BAT_INFO		  , MAX_BAT_COUNT * sizeof(TlvBatInfo), &g_tlvBatInfo[0]    , DT_STRUCT, &g_tlvBatInfo_mirror[0] , Null, 0, 0},
		{"BAT_WORK", &g_tlvBuf_0900Ex[3], TAG_BAT_WORK_PARAM , MAX_BAT_COUNT * sizeof(TlvBatInfo), &g_tlvBatWorkInfo[0], DT_STRUCT, &g_tlvBatWorkInfo_mirror[0], Null, 0, 0},
		//
		{"BAT_TEMP"	, &g_tlvBuf_0900Ex[4], TAG_BAT_TEMP , MAX_BAT_COUNT * sizeof(TlvBatTemp), &g_tlvBatTemp[0], DT_STRUCT, &g_tlvBatTemp_mirror[0], Null, 0, 0},
		{"BAT_FAULT", &g_tlvBuf_0900Ex[5], TAG_BAT_FAULT , MAX_BAT_COUNT * sizeof(TlvBatFault), &g_tlvBatFault[0], DT_STRUCT, &g_tlvBatFault_mirror[0], Null, 0, 0},
	};

	TlvOutMgr_init(&g_jtTlvMgr_0900, g_tlv_0900, GET_ELEMENT_COUNT(g_tlv_0900), 1, True);
}
