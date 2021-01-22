
#include "Common.h"
#include "JtTlv0900.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"
#include "Battery.h"
#include "Dbg.h"


#ifdef CANBUS_MODE_JT808_ENABLE


static TlvOutMgr g_jtTlvMgr_0900;


static uint8	g_smartState = 0x01;
static uint8	g_smartState_mirror = 0x01;

static TlvPMSAttr g_tlvPMSAttr = {0x00} ;
static TlvPMSAttr g_tlvPMSAttr_mirror = {0x00};

static TlvPMSState	g_tlvPMSState = {0x00};	// PMS���в���
static TlvPMSState g_tlvPMSState_mirror = {0x00};

static TlvBatInfo g_tlvBatInfo[MAX_BAT_COUNT] = {0x00};	// ��ػ�����Ϣ
static TlvBatInfo g_tlvBatInfo_mirror[MAX_BAT_COUNT] = {0x00};

static TlvBatWorkInfo g_tlvBatWorkInfo[MAX_BAT_COUNT]= {0x00};	// ��ع�������
static TlvBatWorkInfo g_tlvBatWorkInfo_mirror[MAX_BAT_COUNT]= {0x00};

static TlvBatTemp g_tlvBatTemp[MAX_BAT_COUNT]= {0x00};	// ����¶�����
static TlvBatTemp g_tlvBatTemp_mirror[MAX_BAT_COUNT]= {0x00};

static TlvBatFault g_tlvBatFault[MAX_BAT_COUNT]= {0x00};	// ��ع�������
static TlvBatFault g_tlvBatFault_mirror[MAX_BAT_COUNT]= {0x00};

static TlvBeaconDes g_tlvBeacondes = {0x00};
static TlvBeaconDes g_tlvBeacondes_mirror = {0x00};

extern Battery g_Bat[MAX_BAT_COUNT];

static uint8 	gCacheBuff[256] = {0x00};
static uint8	gCacheLen = 0 ;


//#define		_SWAP_16(x)			SWAP16(x)
#define		_SWAP_16(x)			(x)


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

//void JtTlv0900_updateMirror(const uint8* data, int len)
//{
//	TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, data, len);
//}

/*
	Smart ������״̬
*/
static void _getCurSmart(void){}
extern CfgInfo	g_cfgInfo;
static void _getCurPMSAttr(void)
{
	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);

	g_tlvPMSAttr.PMSId[0] = sn2 >> 24 ;
	g_tlvPMSAttr.PMSId[1] = sn2 >> 16;
	g_tlvPMSAttr.PMSId[2] = sn2 >> 8;
	g_tlvPMSAttr.PMSId[3] = sn2 & 0xFF;
	g_tlvPMSAttr.PMSId[4] = sn1 >> 24;
	g_tlvPMSAttr.PMSId[5] = sn1 >> 16;
	g_tlvPMSAttr.PMSId[6] = sn1 >> 8;
	g_tlvPMSAttr.PMSId[7] = sn1 & 0xFF;
	g_tlvPMSAttr.PMSId[8] = sn0 >> 24;
	g_tlvPMSAttr.PMSId[9] = sn0 >> 16;
	g_tlvPMSAttr.PMSId[10] = sn0 >> 8;
	g_tlvPMSAttr.PMSId[11] = sn0 & 0xFF;
	//Printf("\r\nsID: %08X%08X%08X\r\n",sn2,sn1,sn0);
	//memcpy( g_tlvPMSAttr.PMSId, g_cfgInfo.SN ,12 );
	
	g_tlvPMSAttr.Capacity = 0x02000000;//0x02;	// ֧�����2�����

	g_tlvPMSAttr.HwMainVer = DES_HW_VER_MAIN;
	g_tlvPMSAttr.HwSubVer = DES_HW_VER_SUB;
	g_tlvPMSAttr.FwMainVer = FW_VER_MAIN;
	g_tlvPMSAttr.FwSubver = FW_VER_S1;
	g_tlvPMSAttr.FwRevVer = FW_VER_S2;
	g_tlvPMSAttr.FwBuildNunber =  FW_VER_BUILD ;
	g_tlvPMSAttr.FwBuildNunber = SWAP32( g_tlvPMSAttr.FwBuildNunber );
}


/*
	PMS ����״̬����
*/
static void _getCurPMSState(void)
{
	//uint8_t i = 0;
	//for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	//{
	//	g_tlvPMSState[i].pmsTemp = SWAP16( g_Bat[i].bmsInfo.htemp );	// PMS�¶�
	//	g_tlvPMSState[i].batState = g_Bat[i].presentStatus == BAT_IN ? 0x01 : 0x00 ; // �����λ״̬
		
	//}
	g_tlvPMSState.pmsTemp = 0x00;		// û��ADת��
	g_tlvPMSState.batState = 
		( g_Bat[0].presentStatus == BAT_IN ? 0x01 : 0x00 ) | 
		( g_Bat[1].presentStatus == BAT_IN ? 0x02 : 0x00 ) ;
	//g_tlvPMSState.pmsPer = 0x00;		// С����Ƿ�����
	if( g_pdoInfo.isLowPow &&
		( g_Bat[0].presentStatus == BAT_IN || g_Bat[1].presentStatus == BAT_IN ) )
	{
		g_tlvPMSState.pmsPer = 0x01;
	}
	else
	{
		g_tlvPMSState.pmsPer = 0x00 ;
	}
}

/*
	������� 0x20
*/
static void _getCurBatInfo(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		if( g_Bat[i].presentStatus == BAT_IN )
		{
			g_tlvBatInfo[i].bid[0] = g_Bat[i].bmsID.sn34 & 0xFF ;
			g_tlvBatInfo[i].bid[1] = g_Bat[i].bmsID.sn34 >> 8 ;
			g_tlvBatInfo[i].bid[2] = g_Bat[i].bmsID.sn56 & 0xFF ;
			g_tlvBatInfo[i].bid[3] = g_Bat[i].bmsID.sn56 >> 8 ;
			g_tlvBatInfo[i].bid[4] = g_Bat[i].bmsID.sn78 & 0xFF ;
			g_tlvBatInfo[i].bid[5] = g_Bat[i].bmsID.sn78 >> 8 ;
			//
			g_tlvBatInfo[i].port = i ;	//								// ��ز�λ��
			g_tlvBatInfo[i].nominalVol = _SWAP_16( g_Bat[i].bmsID.bvolt ) ;	// ���ѹ
			g_tlvBatInfo[i].nominalCur = _SWAP_16( g_Bat[i].bmsID.bcap ) ;	// �����

			/*
			Printf("\t%x\n",SWAP16( pBat->bmsID.hwver ) );	// Ӳ���汾
			Printf("\t%x\n", SWAP16( pBat->bmsID.prver ) ); // Э��汾
			//
			Printf("\t%x\n",SWAP16( pBat->bmsID.fwmsv ) );	//�̼����汾
			Printf("\t%x\n", SWAP16( pBat->bmsID.blver ) ); // boot �汾
			Printf("\t%x\n",SWAP16( pBat->bmsID.fwrev ) );	// �̼������汾
			Printf("\t%x\n", ( SWAP16( pBat->bmsID.fwbnh ) << 16 ) | SWAP16( pBat->bmsID.fwbnl )); // �̼�����汾��
			//
			*/
			g_tlvBatInfo[i].HwMainVer = SWAP16(g_Bat[i].bmsID.hwver) >> 8 ;
			g_tlvBatInfo[i].HwSubVer = SWAP16(g_Bat[i].bmsID.hwver) & 0xFF;
			
			g_tlvBatInfo[i].FwMainVer = SWAP16(g_Bat[i].bmsID.fwmsv) >> 8 ;
			g_tlvBatInfo[i].FwSubver = SWAP16(g_Bat[i].bmsID.fwmsv) & 0xFF;
			
			g_tlvBatInfo[i].FwRevVer = SWAP16(g_Bat[i].bmsID.fwrev) & 0xFF;
			g_tlvBatInfo[i].FwBuildNunber = ( SWAP16(g_Bat[i].bmsID.fwbnh) << 16 ) | ( SWAP16(g_Bat[i].bmsID.fwbnl) ) ;
			g_tlvBatInfo[i].FwBuildNunber = SWAP32(g_tlvBatInfo[i].FwBuildNunber);
		}
		else
		{
			memset( &g_tlvBatInfo[i] , 0 , sizeof(TlvBatInfo) );
		}
	}
}

/*
	��ع������� 0x21
*/
static void _getCurBatWork(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		if( g_Bat[i].presentStatus == BAT_IN )
		{
			g_tlvBatWorkInfo[i].bid[0] = g_Bat[i].bmsID.sn34 & 0xFF ;
			g_tlvBatWorkInfo[i].bid[1] = g_Bat[i].bmsID.sn34 >> 8 ;
			g_tlvBatWorkInfo[i].bid[2] = g_Bat[i].bmsID.sn56 & 0xFF ;
			g_tlvBatWorkInfo[i].bid[3] = g_Bat[i].bmsID.sn56 >> 8 ;
			g_tlvBatWorkInfo[i].bid[4] = g_Bat[i].bmsID.sn78 & 0xFF ;
			g_tlvBatWorkInfo[i].bid[5] = g_Bat[i].bmsID.sn78 >> 8 ;
			//
			g_tlvBatWorkInfo[i].soc = SWAP16(g_Bat[i].bmsInfo.soc)/10;					// SOC
			g_tlvBatWorkInfo[i].voltage = _SWAP_16( g_Bat[i].bmsInfo.tvolt );	// ��ѹ
			g_tlvBatWorkInfo[i].current = SWAP16( g_Bat[i].bmsInfo.tcurr )- 30000 ;	// ����
			g_tlvBatWorkInfo[i].current = SWAP16( g_tlvBatWorkInfo[i].current );
			//
			g_tlvBatWorkInfo[i].maxCellVol = _SWAP_16( g_Bat[i].bmsInfo.hvolt );
			g_tlvBatWorkInfo[i].minCellVol = _SWAP_16( g_Bat[i].bmsInfo.lvolt);
			
			g_tlvBatWorkInfo[i].maxCellNum = ( SWAP16( g_Bat[i].bmsInfo.hvnum) ) & 0xFF;
			g_tlvBatWorkInfo[i].minCellNum = ( SWAP16( g_Bat[i].bmsInfo.lvnum) ) & 0xFF;

			// bsminfo����ʱû��
			g_tlvBatWorkInfo[i].maxChgCurr = 0x00;//SWAP16( g_Bat[i].bmsInfo.csop);
			g_tlvBatWorkInfo[i].maxDischgCurr = 0x00;//SWAP16( g_Bat[i].bmsInfo.dsop) ;
			g_tlvBatWorkInfo[i].curWorkFeature = 0x00;//SWAP16( g_Bat[i].bmsInfo.fcap) ;
			//
			g_tlvBatWorkInfo[i].cycCount = _SWAP_16( g_Bat[i].bmsInfo.cycle) ;
		}
		else
		{
			memset( &g_tlvBatWorkInfo[i] , 0 , sizeof(TlvBatWorkInfo) );
		}
	}
}

/*
	����¶����� 0x22
*/
static void _getCurBatTemp(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		if( g_Bat[i].presentStatus == BAT_IN )
		{
			g_tlvBatTemp[i].bid[0] = g_Bat[i].bmsID.sn34 & 0xFF ;
			g_tlvBatTemp[i].bid[1] = g_Bat[i].bmsID.sn34 >> 8 ;
			g_tlvBatTemp[i].bid[2] = g_Bat[i].bmsID.sn56 & 0xFF ;
			g_tlvBatTemp[i].bid[3] = g_Bat[i].bmsID.sn56 >> 8 ;
			g_tlvBatTemp[i].bid[4] = g_Bat[i].bmsID.sn78 & 0xFF ;
			g_tlvBatTemp[i].bid[5] = g_Bat[i].bmsID.sn78 >> 8 ;
			//
			g_tlvBatTemp[i].cMostTemp = _SWAP_16( g_Bat[i].bmsInfo.cmost );	// ���MOS�¶�
			g_tlvBatTemp[i].dMostTemp = _SWAP_16( g_Bat[i].bmsInfo.dmost );	// �ŵ�MOS�¶�
			g_tlvBatTemp[i].fuelTemp = _SWAP_16( g_Bat[i].bmsInfo.pret);	// �������¶�
			g_tlvBatTemp[i].contTemp = _SWAP_16( g_Bat[i].bmsInfo.cont ) ;	// �������¶�
			g_tlvBatTemp[i].batTemp1 = _SWAP_16( g_Bat[i].bmsInfo.btemp[0] );	// ���1�¶�
			g_tlvBatTemp[i].batTemp2 = _SWAP_16( g_Bat[i].bmsInfo.btemp[1] );	// ���2�¶�
			// bmsinof ��ʱû��,�����
			g_tlvBatTemp[i].tvsTemp = 0 ;	// tvs �¶�	
			g_tlvBatTemp[i].fuseTemp = 0 ;	// ����˿�¶�
		}
		else
		{
			memset( &g_tlvBatTemp[i] , 0 , sizeof(TlvBatTemp) );
		}
	}
}

/*
	��ع������� 0x23
*/
static void _getCurBatFault(void)
{
	uint8_t i = 0;
	for( i = 0 ; i < MAX_BAT_COUNT ; i++ )
	{
		if( g_Bat[i].presentStatus == BAT_IN )
		{
			g_tlvBatFault[i].bid[0] = g_Bat[i].bmsID.sn34 & 0xFF ;
			g_tlvBatFault[i].bid[1] = g_Bat[i].bmsID.sn34 >> 8 ;
			g_tlvBatFault[i].bid[2] = g_Bat[i].bmsID.sn56 & 0xFF ;
			g_tlvBatFault[i].bid[3] = g_Bat[i].bmsID.sn56 >> 8 ;
			g_tlvBatFault[i].bid[4] = g_Bat[i].bmsID.sn78 & 0xFF ;
			g_tlvBatFault[i].bid[5] = g_Bat[i].bmsID.sn78 >> 8 ;
			//
			g_tlvBatFault[i].devft1 = _SWAP_16( g_Bat[i].bmsInfo.devft1);	// ����1
			g_tlvBatFault[i].devft2 = _SWAP_16( g_Bat[i].bmsInfo.devft2);
			g_tlvBatFault[i].opft1 = _SWAP_16( g_Bat[i].bmsInfo.opft1);
			g_tlvBatFault[i].opft2 = _SWAP_16( g_Bat[i].bmsInfo.opft2);
			g_tlvBatFault[i].opwarn1 = _SWAP_16( g_Bat[i].bmsInfo.opwarn1);
			g_tlvBatFault[i].opwarn2 = _SWAP_16( g_Bat[i].bmsInfo.opwarn2);
		}
		else
		{
			memset( &g_tlvBatFault[i] , 0 , sizeof(TlvBatFault) );
		}
	}
}




void JtTlv0900_updateStorage( void )		// 
{
	_getCurSmart();
	_getCurPMSAttr();
	_getCurPMSState();
	_getCurBatInfo();
	_getCurBatWork();
	_getCurBatTemp();
	_getCurBatFault();
}

/*
	����Beacon
*/
void JTTlv0900_updateBeacon(Beacon *pBeacon , uint8 bleCnt)
{
	if( !pBeacon || !bleCnt) return ;
	// ���������,�����֮
	// �����û��,�����֮
	// ��������Ѿ�����,������ź�������
	memset( &g_tlvBeacondes , 0 , sizeof(g_tlvBeacondes) );
	g_tlvBeacondes.beaconCnt = bleCnt ;
	memcpy( g_tlvBeacondes.beaconArry , pBeacon , bleCnt * sizeof(Beacon) );
}

void JtTlv0900_Cache( uint8* pdata ,uint8 len )	// ��ȡ��������
{
	gCacheLen = len > sizeof(gCacheBuff) ? sizeof(gCacheBuff) : len ;
	memcpy( gCacheBuff , pdata , gCacheLen );
}

void JtTlv0900_UpdateMirror(void)
{
	if( gCacheLen )
	{
		TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, gCacheBuff, gCacheLen );
		gCacheLen = 0x00 ;
	}
}



int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvMgr_0900, buf, len, tlvCount);
}

void JtTlv0900_init()
{
#define TLV_OUT_COUNT	8
	static TlvOutEx g_tlvBuf_0900Ex[TLV_OUT_COUNT];
	const static TlvOut g_tlv_0900[TLV_OUT_COUNT] =
	{
		{"SMART", &g_tlvBuf_0900Ex[0], TAG_SMART_STATE ,  sizeof(g_smartState), &g_smartState, DT_UINT8, &g_smartState_mirror, Null, 0, 0},

		{"PMS_ARTT", &g_tlvBuf_0900Ex[1], TAG_PSM_ATTR ,  sizeof(TlvPMSAttr), &g_tlvPMSAttr, DT_STRUCT, &g_tlvPMSAttr_mirror, Null, 0, 0},
		
		{"PMS_STATE", &g_tlvBuf_0900Ex[2], TAG_PMS_STATE , sizeof(TlvPMSState), &g_tlvPMSState, DT_STRUCT, &g_tlvPMSState_mirror, Null, 0, 0},
		//
		{"BAT_INFO", &g_tlvBuf_0900Ex[3], TAG_BAT_INFO	, MAX_BAT_COUNT * sizeof(TlvBatInfo), &g_tlvBatInfo[0]    , DT_STRUCT, &g_tlvBatInfo_mirror[0] , Null, 0, 0},
		{"BAT_WORK", &g_tlvBuf_0900Ex[4], TAG_BAT_WORK_PARAM ,MAX_BAT_COUNT * sizeof(TlvBatWorkInfo), &g_tlvBatWorkInfo[0], DT_STRUCT, &g_tlvBatWorkInfo_mirror[0], Null, 0, 0},
		//
		{"BAT_TEMP"	, &g_tlvBuf_0900Ex[5], TAG_BAT_TEMP ,MAX_BAT_COUNT * sizeof(TlvBatTemp), &g_tlvBatTemp[0], DT_STRUCT, &g_tlvBatTemp_mirror[0], Null, 0, 0},
		{"BAT_FAULT", &g_tlvBuf_0900Ex[6], TAG_BAT_FAULT , MAX_BAT_COUNT * sizeof(TlvBatFault), &g_tlvBatFault[0], DT_STRUCT, &g_tlvBatFault_mirror[0], Null, 0, 0},
		{"BEACON_DES", &g_tlvBuf_0900Ex[7], TAG_BEACON_DEC , sizeof(TlvBeaconDes), &g_tlvBeacondes, DT_STRUCT, &g_tlvBeacondes_mirror, Null, 0, 0},
	};

	TlvOutMgr_init(&g_jtTlvMgr_0900, g_tlv_0900, GET_ELEMENT_COUNT(g_tlv_0900), 1, True);
}


#endif //#ifdef CANBUS_MODE_JT808_ENABLE


