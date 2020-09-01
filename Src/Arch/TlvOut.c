
#include "ArchDef.h"
#include "TlvOut.h"
#include "SwTimer.h"

void TlvOut_dump(const TlvOut* pItem)
{
	Printf("%s[0x%X-%d]:\n", pItem->name, pItem->tag, pItem->len);
	Printf("\t"); DUMP_BYTE(pItem->storage, pItem->len);
	Printf("\t"); DUMP_BYTE(pItem->mirror   , pItem->len);
}

void TlvOutMgr_setFlag(const TlvOut* pItem, TlvOutFlag flag)
{
	pItem->pEx->flag |= flag;
}

Bool TlvOutMgr_isChanged(TlvOutMgr* mgr, const TlvOut* pItem)
{
	Bool isChanged = (memcmp(pItem->storage, pItem->mirror, pItem->len) != 0);

	if(pItem->pEx)
	{
		isChanged |= (pItem->pEx->flag & TF_OUT_CHANGED); 
		pItem->pEx->flag &= ~TF_OUT_CHANGED;
		//每秒检查一下是否发生改变
		if(!SwTimer_isTimerOutEx(pItem->pEx->ticks, 1000))
		{
			return False;
		}
		pItem->pEx->timeOutSec++;
		pItem->pEx->ticks = GET_TICKS();
	}
	
	if(isChanged)
	{
		isChanged = pItem->IsChanged ? pItem->IsChanged(mgr, pItem, TE_CHANGED) : isChanged;
	}
	else 
	{
		isChanged = pItem->IsChanged ? pItem->IsChanged(mgr, pItem, TE_UNCHANGED) : isChanged;
	}
	
	if(isChanged && pItem->pEx) 
	{
		pItem->pEx->timeOutSec = 0;
	}
	
	return isChanged;
}

const TlvOut* TlvOutMgr_find(const TlvOut* pItems, int count, uint8 tag)
{
	for(int i = 0; i < count; i++, pItems++)
	{
		if(pItems->tag == tag) return pItems;
	}
	
	return Null;	
}

void TlvOutMgr_resetAll(TlvOutMgr* mgr)
{
	const TlvOut* p = mgr->itemArray;
	for(int i = 0; i < mgr->itemCount; i++, p++)
	{
		memset(p->storage, 0, p->len);
		memset(p->mirror , 0, p->len);
	}
}

int TlvOutMgr_getChanged(TlvOutMgr* mgr, uint8* pBuf, int bufSize)
{
//	int len = 0;
	Bool isChanged = False;
	int offset = 0;
	const TlvOut* p = mgr->itemArray;

	for(int i = 0; i < mgr->itemCount; i++, p++)
	{
		isChanged = TlvOutMgr_isChanged(mgr, p);
		if(isChanged)
		{
			if((offset + p->len + mgr->tagLen + 1) > bufSize) break;
			
			memcpy(&pBuf[offset], &p->tag, mgr->tagLen);
			offset += mgr->tagLen;

			pBuf[offset++] = p->len;			
			memcpy(&pBuf[offset], p->storage, p->len);
			offset += p->len;
		}
	}
	
	return offset;
}

//更新镜像指针值
void TlvOutMgr_updateMirror(TlvOutMgr* mgr, const uint8* pTlvBuf, int bufSize)
{
	const TlvOut* p = Null;
	uint32 tag = 0;
	const uint8* pVal;
	for(int i = 0; i < bufSize; )
	{
		pVal = &pTlvBuf[mgr->tagLen + 1];
		memcpy(&tag, pTlvBuf, mgr->tagLen);
		p = TlvOutMgr_find(mgr->itemArray, mgr->itemCount, tag);
		if(p == Null) break;

		if (p->idLen)
		{
			uint8_t* storage = (uint8*)p->storage;
			if(memcmp(&storage[p->idInd], &pVal[p->idInd], p->idLen) == 0)
			{
				memcpy(p->mirror, pVal, p->len);
			}
		}
		else
		{
			memcpy(p->mirror, pVal, p->len);
		}

		pTlvBuf += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen];
	}
}

void TlvOutMgr_init(TlvOutMgr* mgr, const TlvOut* items, int itemCount,  int tagLen)
{
	mgr->itemArray = items;
	mgr->itemCount = itemCount;
	mgr->tagLen = tagLen;
}

