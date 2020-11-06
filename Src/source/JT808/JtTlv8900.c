
#include "Common.h"
#include "JtTlv8900.h"
#include "TlvIn.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"

static TlvInMgr g_jtTlvInMgr_8900;

static Jt8900 g_Jt8900 ;

TlvInEventRc JtTlv8900_Event(TlvInMgr* mgr, const TlvIn* pItem, TlvInEvent ev)
{
	if( ev == TE_CHANGED_AFTER )
	{
		switch ( pItem->tag )
		{
			case TAG_ACTIVE :
				g_cfgInfo.isActive = g_Jt8900.mAcitveState;//*pItem->storage;
				NvdsUser_Write(NVDS_CFG_INFO);
				break ;
			case TAG_SET_ACC_STATE :
				g_pdoInfo.isRemoteAccOn = g_Jt8900.mAccState;//*pItem->storage ;
				if(g_pdoInfo.isRemoteAccOn)
				{
					Pms_postMsg(PmsMsg_accOn, 0, 0);
					LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOn,0);
				}
				else
				{
					Pms_postMsg(PmsMsg_accOff, 0, 0);
					LOG_TRACE1(LogModuleID_SYS, SYS_CATID_COMMON, 0, SysEvtID_SetAccOn,0);
				}
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_WHELL_LOCK :
				g_pdoInfo.isWheelLock  = g_Jt8900.mWhellState;//*pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_CABIN_LOCK :
				g_pdoInfo.isCanbinLock = g_Jt8900.mCabState;//*pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_POWER_OFF :
				g_pdoInfo.IsForbidDischarge = g_Jt8900.mPowerOff;//*pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_BAT_IDEN_EN :
				g_pdoInfo.IsBatVerifyEn = g_Jt8900.mBatIDEnable;//*pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_BAT_ALAM_EN :
				g_pdoInfo.IsAlarmMode = g_Jt8900.mBatAlamEnable;//*pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_BAT_BID :
				//g_pdoInfo.BatVerifyRet = *pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			default :break ;
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JtTlv8900_proc(const uint8* data, int len)
{
	TlvInMgr_updateStorage(&g_jtTlvInMgr_8900, data, len);

	return UTP_EVENT_RC_SUCCESS;
}

void JtTlv8900_init()
{
	const static TlvIn g_tlvIn_8900[] =
	{
		{"ACT_STATE"		, TAG_ACTIVE,			1, &g_Jt8900.mAcitveState ,DT_UINT8,Null,Null },
		{"SET_ACC  "		, TAG_SET_ACC_STATE,	1, &g_Jt8900.mAccState,DT_UINT8,Null,Null},
		{"SET_WHELL_LOCK"	, TAG_SET_WHELL_LOCK,	1, &g_Jt8900.mWhellState,DT_UINT8,Null,Null},
		{"SET_CABIN_LOCK"	, TAG_SET_CABIN_LOCK,	1, &g_Jt8900.mCabState,DT_UINT8,Null,Null},
		{"SET_POWER_OFF"	, TAG_SET_POWER_OFF,	1, &g_Jt8900.mPowerOff,DT_UINT8,Null,Null},
		{"SET_BAT_IDEN"		, TAG_SET_BAT_IDEN_EN,	1, &g_Jt8900.mBatIDEnable,DT_UINT8,Null,Null},
		{"SET_BAT_ALAMEN"	, TAG_SET_BAT_ALAM_EN,	1, &g_Jt8900.mBatAlamEnable,DT_UINT8,Null,Null},
		{"SET_BAT_BID"		, TAG_SET_BAT_BID,		1, (uint8_t*)&g_Jt8900.mBatVerify,DT_STRUCT,Null,Null},
	};
	TlvInMgr_init(&g_jtTlvInMgr_8900, g_tlvIn_8900, GET_ELEMENT_COUNT(g_tlvIn_8900), 1, (TlvInEventFn)JtTlv8900_Event, False);
}



