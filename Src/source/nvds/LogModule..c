#include "Common.h"
#include "LogModule.h"

void LogModule_Add(LogModule* pHead, const LogModule* pNew)
{
	List_Add((List*)pHead, (List*)pNew);
}

LogModule* LogModule_Find(LogModule* pHead, uint8 moduleId)
{
	List* p = (List*)pHead;
	for (p; p->next != Null; p = p->next)
	{
		LogModule* pMod = (LogModule*)p;
		if (moduleId >= pMod->moduleIdMin && moduleId <= pMod->moduleIdMax)
		{
			return pMod;
		}
	}
	return Null;
}

void LogModule_Init(LogModule* pMod,
	uint8		moduleIdMin	,
	uint8		moduleIdMax	,
	LogToStrFn	EventIdToStr,
	LogToStrFn	ValueToStr	,
	LogToStrFn	MoudleIdToStr,
	LogToStrFn	SubIdToStr	,
	LogToStrFn	CatIdToStr
)
{
	pMod->moduleIdMin	= moduleIdMin	;
	pMod->moduleIdMax	= moduleIdMax	;
	pMod->EventIdToStr	= EventIdToStr	;
	pMod->ValueToStr	= ValueToStr	;
	pMod->MoudleIdToStr = MoudleIdToStr	;
	pMod->SubIdToStr	= SubIdToStr	;
	pMod->CatIdToStr	= CatIdToStr	;
}
