
#ifndef _LOG_MODULE_H_
#define _LOG_MODULE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "list.h"
#include "Log.h"

typedef const char* (*LogToStrFn)(const struct _LogItem* pItem, char* buf, int size);
typedef struct _LogModule
{
	List* base;

	uint8		moduleIdMin;	//模块ID最小值，记录触发的log的模块ID
	uint8		moduleIdMax;	//模块ID最大值，记录触发的log的模块ID
	LogToStrFn	EventIdToStr;	//事件ID转换到字符串函数指针
	LogToStrFn	ValueToStr;		//事件值转换到字符串函数指针
	LogToStrFn	MoudleIdToStr;	//模块ID转换到字符串函数指针
	LogToStrFn	SubIdToStr;		//事件子ID转换到字符串函数指针
	LogToStrFn	CatIdToStr;		//事件类别ID转换到字符串函数指针
}LogModule;

void LogModule_Init(LogModule* pMod,
		uint8		moduleIdMin,
		uint8		moduleIdMax,
		LogToStrFn	EventIdToStr,
		LogToStrFn	ValueToStr,
		LogToStrFn	MoudleIdToStr,
		LogToStrFn	SubIdToStr,
		LogToStrFn	CatIdToStr
	);
void LogModule_Add(LogModule* pHead, const LogModule* pNew);
LogModule* LogModule_Find(LogModule* pHead, uint8 moduleId);

#ifdef __cplusplus
}
#endif

#endif
