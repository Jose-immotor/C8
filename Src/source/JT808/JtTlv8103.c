
#include "Common.h"
#include "JtTlv8103.h"
#include "TlvIn.h"
#include "TlvOut.h"
#include "jt808.h"

TlvInMgr  g_jtTlvInMgr_8103;

void JtTlv8103_Dump()
{
	TlvInMgr* mgr = &g_jtTlvInMgr_8103;
	const TlvIn* p = mgr->itemArray;

	for (int i = 0; i < mgr->itemCount; i++, p++)
	{
		TlvInMgr_dump(p, mgr->tagLen, p->dt);
	}
}

static TlvInEventRc JtTlv8103_InEvent(TlvInMgr* mgr, const TlvIn* pItem, TlvInEvent ev)
{
	if (ev == TE_CHANGED_AFTER)
	{

	}

	return TERC_SUCCESS;
}

void JtTlv8103_updateStorage(const uint8* data, int len)
{
	TlvInMgr_updateStorage(&g_jtTlvInMgr_8103, data, len);
}

int JtTlv8103_getChanged(uint8* buf, int len, uint8* tlvCount)
{
	//return TlvOutMgr_getChanged(&g_jtTlvOutMgr_8103, buf, len, tlvCount);
}

void JtTlv8103_init()
{
	//mirror obj for g_Jt.cfgParam

	#define TLV_8103_COUNT 12
	const static TlvIn g_tlvIn_8103[TLV_8103_COUNT] =
	{
		{"HB_INTERVAL"		, TAG_HB_INTERVAL	 , 4  , (uint8*)&g_Jt.cfgParam.hbIntervalS   , DT_UINT32},
		{"TCP_RSP_TIME "	, TAG_TCP_RSP_TIME   , 4  , (uint8*)&g_Jt.cfgParam.tcpWaitRspTime, DT_UINT32},
		{"TCP_RETX_COUNT"	, TAG_TCP_RETX_COUNT , 4  , (uint8*)&g_Jt.cfgParam.tcpReTxCount  , DT_UINT32},
		{"MAIN_SVR_URL"		, TAG_MAIN_SVR_URL   , 256, (uint8*)&g_Jt.cfgParam.mainSvrUrl    , DT_STRING},
		{"MAIN_SVR_PORT"	, TAG_MAIN_SVR_PORT  , 4  , (uint8*)& g_Jt.cfgParam.mainSvrPort  , DT_UINT32},

		{"LOC_REPORT_WAY"	, TAG_LOC_REPORT_WAY	, 4,(uint8*)& g_Jt.cfgParam.locReportWay			, DT_UINT32},
		{"LOC_REPORT_PLAN"	, TAG_LOC_REPORT_PLAN	, 4,(uint8*)& g_Jt.cfgParam.locReportPlan			, DT_UINT32},
		{"SLEEP_LOC_INTERVAL", TAG_SLEEP_LOC_INTERVAL, 4,(uint8*)& g_Jt.cfgParam.sleepLocReportInterval	, DT_UINT32},
		{"URG_LOC_INTERVAL"	, TAG_URG_LOC_INTERVAL	, 4,(uint8*)& g_Jt.cfgParam.urgLocReportInterval	, DT_UINT32},
		{"DEF_LOC_INTERVA"  , TAG_DEF_LOC_INTERVAL	, 4,(uint8*)& g_Jt.cfgParam.defLocReportInterval	, DT_UINT32},

		{"DEVICE_TYPE"		, TAG_DEVICE_TYPE	   , 4  , (uint8*)& g_Jt.cfgParam.devType		, DT_UINT32},
		{"FACTORY_CFG_FLA"	, TAG_FACTORY_CFG_FLAG , 4  , (uint8*)& g_Jt.cfgParam.factoryFlag	, DT_UINT32},
	};														  
	TlvInMgr_init(&g_jtTlvInMgr_8103, g_tlvIn_8103, GET_ELEMENT_COUNT(g_tlvIn_8103), 2, (TlvInEventFn)JtTlv8103_InEvent, True);


	//TlvOutMgr g_jtTlvOutMgr_8103;
	//static JtDevCfgParam g_cfgParam_mirror;
	//static TlvOutEx g_tlvOutEx_8103[TLV_8103_COUNT];
	//const TlvOut g_tlvOut_8103[TLV_8103_COUNT] =
	//{
	//	{"HB_INTERVAL"		,Null, TAG_HB_INTERVAL   , 4, (uint8*)& g_Jt.cfgParam.hbIntervalS	 , (uint8*)& g_cfgParam_mirror.hbIntervalS	},
	//	{"TCP_RSP_TIME"		,Null, TAG_TCP_RSP_TIME  , 4, (uint8*)& g_Jt.cfgParam.tcpWaitRspTime , (uint8*)& g_cfgParam_mirror.tcpWaitRspTime },
	//	{"TCP_RETX_COUNT"	,Null, TAG_TCP_RETX_COUNT, 4, (uint8*)& g_Jt.cfgParam.tcpReTxCount   , (uint8*)& g_cfgParam_mirror.tcpReTxCount   },
	//	{"MAIN_SVR_URL"		,Null, TAG_MAIN_SVR_URL  , 4, (uint8*)& g_Jt.cfgParam.mainSvrUrl     , (uint8*)& g_cfgParam_mirror.mainSvrUrl     },
	//	{"MAIN_SVR_PORT"	,Null, TAG_MAIN_SVR_PORT , 4, (uint8*)& g_Jt.cfgParam.mainSvrPort     , (uint8*)&g_cfgParam_mirror.mainSvrPort    },
	//};
	//TlvOutMgr_init(&g_jtTlvOutMgr_8103, g_tlvOut_8103, GET_ELEMENT_COUNT(g_tlvOut_8103), 2);
}
