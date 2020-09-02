/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"
#include "TlvIn.h"


void TlvInMgr_dump(const TlvIn* pItem)
{
	Printf("%s[0x%X-%d]:\n", pItem->name, pItem->tag, pItem->len);
	Printf("\t"); DUMP_BYTE(pItem->storage, pItem->len);
}

const TlvIn* TlvInMgr_find(const TlvIn* pItems, int count, uint8 tag)
{
	for(int i = 0; i < count; i++, pItems++)
	{
		if(pItems->tag == tag) return pItems;
	}
	
	return Null;	
}

void TlvInMgr_resetAll(TlvInMgr* mgr)
{
	const TlvIn* p = mgr->itemArray;
	for(int i = 0; i < mgr->itemCount; i++, p++)
	{
		memset(p->storage, 0, p->len);
	}
}

TlvInEventRc TlvInMgr_event(TlvInMgr* mgr, const TlvIn* p, TlvInEvent ev)
{
	TlvInEventRc rc = TERC_SUCCESS;

	if (mgr->Event) 
		rc = mgr->Event(mgr->pObj, p, ev);

	if (rc == TERC_SUCCESS && p->Event)
		rc = p->Event(mgr->pObj, p, ev);

	return rc;
}

//更新存储指针值
void TlvInMgr_updateStroage(TlvInMgr* mgr, const uint8* pTlvBuf, int bufSize)
{
	const TlvIn* p = Null;

	uint32 tag = 0;
	for (int i = 0; i < bufSize; )
	{
		memcpy(&tag, pTlvBuf, mgr->tagLen);
		p = TlvInMgr_find(mgr->itemArray, mgr->itemCount, tag);
		if (p)
		{
			if (memcmp(p->storage, &pTlvBuf[mgr->tagLen + 1], pTlvBuf[mgr->tagLen]) != 0)
			{
				if (TERC_SUCCESS == TlvInMgr_event(mgr, p, TE_CHANGED_BEFORE))
				{
					memcpy(p->storage, &pTlvBuf[mgr->tagLen + 1], p->len);
					TlvInMgr_event(mgr, p, TE_CHANGED_AFTER);
				}
			}
			TlvInMgr_event(mgr, p, TE_UPDATE_DONE);
		}

		pTlvBuf += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen];
	}
}

void TlvInMgr_init(TlvInMgr* mgr, const TlvIn* items, int itemCount, uint8 tagLen, TlvInEventFn Event)
{
	mgr->itemArray = items;
	mgr->itemCount = itemCount;
	mgr->Event = Event;
	mgr->tagLen = tagLen;
}
