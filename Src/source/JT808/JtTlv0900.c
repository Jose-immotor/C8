
#include "Common.h"
#include "JtTlv0900.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"

static TlvOutMgr g_jtTlvMgr_0900;

static TlvBatInfo g_tlvBatInfo[MAX_BAT_COUNT];
static TlvBatInfo g_tlvBatInfo_mirror[MAX_BAT_COUNT];

static TlvBatWorkInfo g_tlvBatWorkInfo[MAX_BAT_COUNT];
static TlvBatWorkInfo g_tlvBatWorkInfo_mirror[MAX_BAT_COUNT];

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

int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvMgr_0900, buf, len, tlvCount);
}

void JtTlv0900_init()
{
#define TLV_OUT_COUNT 4
	static TlvOutEx g_tlvBuf_0900Ex[TLV_OUT_COUNT];
	static const TlvOut g_tlv_0900[TLV_OUT_COUNT] =
	{
		{"BAT_INFO"	, &g_tlvBuf_0900Ex[0], TAG_BAT_INFO		  , sizeof(TlvBatInfo), &g_tlvBatInfo[0]    , DT_STRUCT, &g_tlvBatInfo_mirror[0]	, Null, 0, 6},
		{"BAT_WORK"	, &g_tlvBuf_0900Ex[1], TAG_BAT_WORK_PARAM , sizeof(TlvBatInfo), &g_tlvBatWorkInfo[0], DT_STRUCT, &g_tlvBatWorkInfo_mirror[0], Null, 0, 6},
	};

	TlvOutMgr_init(&g_jtTlvMgr_0900, g_tlv_0900, GET_ELEMENT_COUNT(g_tlv_0900), 1, True);
}
