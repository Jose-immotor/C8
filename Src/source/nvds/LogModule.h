
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

	uint8		moduleIdMin;	//ģ��ID��Сֵ����¼������log��ģ��ID
	uint8		moduleIdMax;	//ģ��ID���ֵ����¼������log��ģ��ID
	LogToStrFn	EventIdToStr;	//�¼�IDת�����ַ�������ָ��
	LogToStrFn	ValueToStr;		//�¼�ֵת�����ַ�������ָ��
	LogToStrFn	MoudleIdToStr;	//ģ��IDת�����ַ�������ָ��
	LogToStrFn	SubIdToStr;		//�¼���IDת�����ַ�������ָ��
	LogToStrFn	CatIdToStr;		//�¼����IDת�����ַ�������ָ��
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
