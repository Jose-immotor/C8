
#include "Common.h"
#include "JtTlv8103.h"
#include "TlvIn.h"
#include "TlvOut.h"
#include "jt808.h"

TlvInMgr  g_jtTlvInMgr_8103;
TlvOutMgr g_jtTlvOutMgr_8103;

//#define		_JT808_DEGMSG 		Printf

#define		_NETWORK_SERVER_URL		"t-ec-netty-g2.ehuandian.net"//47.107.191.163"//"d-ec-netty-g2.ehuandian.net"
#define		_NETWORK_SERVER_PORT	9006

// 读取返回的数据
static JtDevCfgParam g_cfgParam_mirror ;
// 默认的数据(以后要保存到Flash)
static JtDevCfgParam g_cfgParam = 
{
	.hbIntervalS = 30 ,		
	.tcpWaitRspTime = 5 ,
	.tcpReTxCount = 3 ,
	.mainSvrPort = _NETWORK_SERVER_PORT,
	.locReportWay = 30,
	.locReportPlan = 30,
	.sleepLocReportInterval = 30,
	.urgLocReportInterval = 30 ,
	.defLocReportInterval = 30 ,
	.gpsSampFre = 2,
	.devType = 1 ,
	.factoryFlag = 0 ,
};

void JtTlv8103IP_Dump()
{	
	Printf("\tIpAddr:%s, Port=%d.\n", g_cfgParam.mainSvrUrl, g_cfgParam.mainSvrPort);
}


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

void JtTlv8103_updateMirror(const uint8* data, int len)
{
	TlvOutMgr_updateMirror(&g_jtTlvOutMgr_8103,data, len );

	//
	//_JT808_DEGMSG("Intervals:%d\r\n",g_cfgParam_mirror.hbIntervalS);
	//_JT808_DEGMSG("tcpWaitRspTime:%d\r\n",g_cfgParam_mirror.tcpWaitRspTime);
	//_JT808_DEGMSG("tcpReTxCount:%d\r\n",g_cfgParam_mirror.tcpReTxCount);
	//_JT808_DEGMSG("mainSvrUrl:%s\r\n",g_cfgParam_mirror.mainSvrUrl);
	//_JT808_DEGMSG("mainSvrPort:%d\r\n",g_cfgParam_mirror.mainSvrPort);
	
	//_JT808_DEGMSG("locReportWay:%d\r\n",g_cfgParam_mirror.locReportWay);
	//_JT808_DEGMSG("locReportPlan:%d\r\n",g_cfgParam_mirror.locReportPlan);
	//_JT808_DEGMSG("sleepLocReportInterval:%d\r\n",g_cfgParam_mirror.sleepLocReportInterval);
	//_JT808_DEGMSG("urgLocReportInterval:%d\r\n",g_cfgParam_mirror.urgLocReportInterval);
	//_JT808_DEGMSG("defLocReportInterval:%d\r\n",g_cfgParam_mirror.defLocReportInterval);
	//_JT808_DEGMSG("devType:%d\r\n",g_cfgParam_mirror.devType);
	//_JT808_DEGMSG("factoryFlag:%d\r\n",g_cfgParam_mirror.factoryFlag);
}

void JtTlv8103_updateStorage(const uint8* data, int len)		// 
{
	TlvInMgr_updateStorage(&g_jtTlvInMgr_8103, data, len );
}

int JtTlv8103_getChanged(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvOutMgr_8103, buf, len, tlvCount);
}





void JtTlv8103_init()
{
	//mirror obj for g_Jt.cfgParam

	#define TLV_8103_COUNT 13
	const static TlvIn g_tlvIn_8103[TLV_8103_COUNT] =
	{
		{"HB_INTERVAL"		, TAG_HB_INTERVAL	 , 4  , (uint8*)&g_cfgParam.hbIntervalS   , DT_UINT32},
		{"TCP_RSP_TIME "	, TAG_TCP_RSP_TIME   , 4  , (uint8*)&g_cfgParam.tcpWaitRspTime, DT_UINT32},
		{"TCP_RETX_COUNT"	, TAG_TCP_RETX_COUNT , 4  , (uint8*)&g_cfgParam.tcpReTxCount  , DT_UINT32},
		{"MAIN_SVR_URL"		, TAG_MAIN_SVR_URL   , 256, (uint8*)&g_cfgParam.mainSvrUrl    , DT_STRING},
		{"MAIN_SVR_PORT"	, TAG_MAIN_SVR_PORT  , 4  , (uint8*)&g_cfgParam.mainSvrPort  , DT_UINT32},

		{"LOC_REPORT_WAY"	, TAG_LOC_REPORT_WAY	, 4,(uint8*)&g_cfgParam.locReportWay			, DT_UINT32},
		{"LOC_REPORT_PLAN"	, TAG_LOC_REPORT_PLAN	, 4,(uint8*)&g_cfgParam.locReportPlan			, DT_UINT32},
		{"SLEEP_LOC_INTERVAL", TAG_SLEEP_LOC_INTERVAL, 4,(uint8*)&g_cfgParam.sleepLocReportInterval	, DT_UINT32},
		{"URG_LOC_INTERVAL"	, TAG_URG_LOC_INTERVAL	, 4,(uint8*)&g_cfgParam.urgLocReportInterval	, DT_UINT32},
		{"DEF_LOC_INTERVA"  , TAG_DEF_LOC_INTERVAL	, 4,(uint8*)&g_cfgParam.defLocReportInterval	, DT_UINT32},
		{"GPS_SAMPLING_FREQUENCY"  , TAG_GPS_SAMPL_FREQUENCY	, 4,(uint8*)&g_cfgParam.gpsSampFre	, DT_UINT32},

		{"DEVICE_TYPE"		, TAG_DEVICE_TYPE	   , 4  , (uint8*)&g_cfgParam.devType		, DT_UINT32},
		{"FACTORY_CFG_FLA"	, TAG_FACTORY_CFG_FLAG , 4  , (uint8*)&g_cfgParam.factoryFlag	, DT_UINT32},
	};														  
	TlvInMgr_init(&g_jtTlvInMgr_8103, g_tlvIn_8103, GET_ELEMENT_COUNT(g_tlvIn_8103), 2, (TlvInEventFn)JtTlv8103_InEvent, /*True*/False);

	const static TlvOut g_tlvOut_8103[TLV_8103_COUNT] =
	{
		{"HB_INTERVAL"		,Null, TAG_HB_INTERVAL	 , 4  , (uint8*)&g_cfgParam.hbIntervalS   ,DT_UINT32, (uint8*)& g_cfgParam_mirror.hbIntervalS ,Null ,0,0},
		{"TCP_RSP_TIME "	,Null, TAG_TCP_RSP_TIME   , 4  , (uint8*)&g_cfgParam.tcpWaitRspTime,DT_UINT32, (uint8*)& g_cfgParam_mirror.tcpWaitRspTime,Null ,0,0},
		{"TCP_RETX_COUNT"	,Null, TAG_TCP_RETX_COUNT , 4  , (uint8*)&g_cfgParam.tcpReTxCount  ,DT_UINT32, (uint8*)& g_cfgParam_mirror.tcpReTxCount,Null ,0,0},
		{"MAIN_SVR_URL"		,Null, TAG_MAIN_SVR_URL   , 256, (uint8*)&g_cfgParam.mainSvrUrl    ,DT_STRING, (uint8*)& g_cfgParam_mirror.mainSvrUrl,Null ,0,0},
		{"MAIN_SVR_PORT"	,Null, TAG_MAIN_SVR_PORT  , 4  , (uint8*)&g_cfgParam.mainSvrPort  ,DT_UINT32, (uint8*)& g_cfgParam_mirror.mainSvrPort,Null ,0,0},

		{"LOC_REPORT_WAY"	,Null, TAG_LOC_REPORT_WAY	, 4,(uint8*)&g_cfgParam.locReportWay	,DT_UINT32, (uint8*)& g_cfgParam_mirror.locReportWay,Null ,0,0},
		{"LOC_REPORT_PLAN"	,Null, TAG_LOC_REPORT_PLAN	, 4,(uint8*)&g_cfgParam.locReportPlan	,DT_UINT32, (uint8*)& g_cfgParam_mirror.locReportPlan,Null ,0,0},
		{"SLEEP_LOC_INTERVAL",Null, TAG_SLEEP_LOC_INTERVAL, 4,(uint8*)&g_cfgParam.sleepLocReportInterval,DT_UINT32, (uint8*)& g_cfgParam_mirror.sleepLocReportInterval,Null ,0,0},
		{"URG_LOC_INTERVAL"	,Null, TAG_URG_LOC_INTERVAL	, 4,(uint8*)&g_cfgParam.urgLocReportInterval,DT_UINT32	, (uint8*)& g_cfgParam_mirror.urgLocReportInterval,Null ,0,0},
		{"DEF_LOC_INTERVA"  ,Null, TAG_DEF_LOC_INTERVAL	, 4,(uint8*)&g_cfgParam.defLocReportInterval,DT_UINT32	, (uint8*)& g_cfgParam_mirror.defLocReportInterval,Null ,0,0},
		{"GPS_SAMPLING_FREQUENCY"	,Null, TAG_GPS_SAMPL_FREQUENCY , 4,(uint8*)&g_cfgParam.gpsSampFre,DT_UINT32 , (uint8*)& g_cfgParam_mirror.gpsSampFre,Null ,0,0},

		{"DEVICE_TYPE"		,Null, TAG_DEVICE_TYPE	   , 4  , (uint8*)&g_cfgParam.devType,DT_UINT32		, (uint8*)& g_cfgParam_mirror.devType,Null ,0,0},
		{"FACTORY_CFG_FLA"	,Null, TAG_FACTORY_CFG_FLAG , 4  , (uint8*)&g_cfgParam.factoryFlag,DT_UINT32	, (uint8*)& g_cfgParam_mirror.factoryFlag,Null ,0,0},
	};
	TlvOutMgr_init(&g_jtTlvOutMgr_8103, g_tlvOut_8103, GET_ELEMENT_COUNT(g_tlvOut_8103), 2 , True);

	// URL
	strcpy( g_cfgParam.mainSvrUrl , _NETWORK_SERVER_URL);
	g_cfgParam.mainSvrPort = _NETWORK_SERVER_PORT;
}



