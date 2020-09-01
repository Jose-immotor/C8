
#include "Common.h"
#include "JtTlv8900.h"
#include "TlvIn.h"
#include "TlvOut.h"
#include "NvdsUser.h"
#include "Pms.h"

static TlvInMgr g_jtTlvInMgr_8900;

TlvInEventRc JtTlv8900_Event(TlvInMgr* mgr, const TlvIn* pItem, TlvInEvent ev)
{
	if (ev == TE_CHANGED_AFTER)
	{
		if (pItem->tag == TAG_ACTIVE)
		{
			g_cfgInfo.isActive = *pItem->storage;
			NvdsUser_Write(NVDS_CFG_INFO);
		}
		else if (pItem->tag == TAG_SET_ACC_STATE) g_pdoInfo.isRemoteAccOn = *pItem->storage;
		else if (pItem->tag == TAG_SET_WHELL_LOCK) g_pdoInfo.isWheelLock  = *pItem->storage;
		else if (pItem->tag == TAG_SET_CABIN_LOCK) g_pdoInfo.isCanbinLock = *pItem->storage;

		NvdsUser_Write(NVDS_PDO_INFO);
	}
}

UTP_EVENT_RC JtTlv8900_proc(const uint8* data, int len)
{
	TlvInMgr_updateStroage(&g_jtTlvInMgr_8900, data, len);

	return UTP_EVENT_RC_SUCCESS;
}

void JtTlv8900_init()
{
	#define RX_TLV_COUNT 4
	static uint8 g_tlvInBuf_8900[RX_TLV_COUNT];
	const TlvIn g_tlvIn_8900[RX_TLV_COUNT] =
	{
		{"ACT_STATE"		, TAG_ACTIVE        , 1, &g_tlvInBuf_8900[0]},
		{"SET_ACC  "		, TAG_SET_ACC_STATE , 1, &g_tlvInBuf_8900[1]},
		{"SET_WHELL_LOCK"	, TAG_SET_WHELL_LOCK, 1, &g_tlvInBuf_8900[2]},
		{"SET_CABIN_LOCK"	, TAG_SET_CABIN_LOCK, 1, &g_tlvInBuf_8900[3]},
	};
	TlvInMgr_init(&g_jtTlvInMgr_8900, g_tlvIn_8900, GET_ELEMENT_COUNT(g_tlvIn_8900), 1, JtTlv8900_Event);
}
