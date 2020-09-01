#ifndef __LOG_H_
#define __LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

//#include "common.h"
#include "Record.h"

#define CFG_LOG

#ifdef CFG_LOG

#define	LOG_STR_MAX_SIZE	200
#define LOG_INVALID_IND		0xFFFFFFFF

#define	ET_CATID_UNDEFIND 0	//û�����CAT ID
#define	ET_SUBID_UNDEFIND 0	//û�����SUB ID

#define LOG_TRACE1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}
#define LOG_TRACE4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_TRACE , eventId, _val);}

#define LOG_WARNING1(moduleId, catId, subId, eventId, __val) 		{ uint32 _val = (__val)					    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		    ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING3(moduleId, catId, subId, eventId, asU16H, asU16L) { uint32 _val = (asU16H << 16) | asU16L   ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}
#define LOG_WARNING4(moduleId, catId, subId, eventId, b3, b2, b1, b0) { uint32 _val = AS_UINT32(b3, b2, b1, b0) ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_WARNING , eventId, _val);}

#define LOG_ERROR1(moduleId, catId, subId, eventId, __val) 			{ uint32 _val = (__val)					 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR2(moduleId, catId, subId, eventId, b1, b0) 		{ uint32 _val = AS_UINT16(b1, b0)		 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR3(moduleId, catId, subId, eventId, asU16H, asU16L)	{ uint32 _val = (asU16H << 16) | asU16L	 ; Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}
#define LOG_ERROR4(moduleId, catId, subId, eventId, b3, b2, b1, b0)	{ uint32 _val = AS_UINT32(b3, b2, b1, b0); Log_Write(g_plogMgr, moduleId, catId, subId, LT_ERROR , eventId, _val);}

typedef Bool(*LogItemFilterFn)(void* pItem, uint32 pParam);

typedef enum _LogType
{
	  LT_TRACE
	, LT_WARNING
	, LT_ERROR
}LogType;

//typedef const char* (*LogFmtFn)(const char* str);

//��־�¼��ṹ����
typedef struct _LogEvt
{
	//uint8	moudleId;	//
	uint8	catId;				//���ID
	uint8	subId;				//�����ID
	uint8	eventId;			//�¼�ID
	const char* pEventIdStr;	//�¼�ID��������ӡ���ʱȡ��eventId

/************************************************************
	pFmt�����ȱ���<=LOG_STR_MAX_SIZE��֧�����¸�ʽ��
		"%1B"		//���val.BIT[0], ��ֵ��ʾΪ10����
		"%1BX"		//���������һ��
		"%0-1B"		//���val.BIT[0-1], ��ֵ��ʾΪ10����
		"%2-6BX"	//���val.BIT[2-6], ��ֵ��ʾΪ16����
		"%7-10B{1:Title1|2:Title2|...}"//���val.BIT[7-10]����ֵʹ�ô������ڵ��ַ�������,�����ŵ��ַ������Ȳ��ܳ���128���ֽڡ�
		"%7-10BX{1:Title1|2:Title2|...}"//���������һ����
************************************************************/
	const char* pEventValueStrFmt;	
}LogEvt;

struct _LogItem;
typedef const char* (*LogToStrFn)(const struct _LogItem* pItem, char* buf, int size);
typedef struct _LogModule
{
	uint8		moduleId;	//ģ��ID
	const char* name;			//ģ������

	const LogEvt* logEvtArray;	//��־ʱ������
	int			logEvtArrayCount;	//��־�¼�����Ԫ������
}LogModule;

#pragma anon_unions
#pragma pack(1) 
//�ýṹ���ֽ��ձ���ż��������ᵼ�´洢�����½�
typedef struct _LogItem
{	
	uint32 dateTime;	//����ʱ�䣬��1971��1��1�տ�ʼ�����ڵ�����

	uint8 version : 1;	//�汾��
	uint8 logType : 2;	//�ο� LogType
	uint8 reserved : 2;
	uint8 catId : 3;	//���ID������subId���������

	uint8 moduleId;		//ģ��ID����¼������log��ģ��ID
	uint8 subId;		//ģ����ID

	uint8 eventId;	//�¼�ID

	union
	{
		uint8 data[4];	//�¼�����
		uint32 asUint32;
	};
}LogItem;
#pragma pack() 

typedef enum _LOG_EVENT
{
	LOG_WRITE_BEFORE,	//before of writing log
	LOG_WRITE_AFTER,	//before of writing log
	LOG_DUMP,			//dump log record
}LOG_EVENT;

struct _LogMgr;
typedef Bool(*LogEventFun)(struct _LogMgr* p, const LogItem * pItem, LOG_EVENT ev);
typedef uint32(*GetCurrentSecondsFn)();
typedef Bool(*LogFilterFun)(struct _LogMgr* p, const LogItem * pItem);

//Log���ýṹ�����ڳ�ʼ��logģ��Ĵ洢���ʹ�ӡ
typedef struct LogCfg
{
	//LOG�İ汾�ţ�����У��ʹ洢�İ汾���Ƿ�һ��
	uint8 logVersion;

	//Logģ�������
	const LogModule* moduleArray;
	int moduleCount;

	//Log���¼�����������ΪNull
	LogEventFun Event;

	//��ȡRTC������������ΪNull
	GetCurrentSecondsFn GetCurSec;
}LogCfg;

typedef struct _LogMgr
{
	const LogCfg* cfg;
	const RecordCfg* recordCfg;
	Record record;

	//�¼�ֵ��Buffer
	char eventValueBuf[LOG_STR_MAX_SIZE+16];
	//�¼�ֵ��Buffer����
	int eventValueBufSize;

	char catIdBuf[16];
	int catIdBufSize;

	char subIdBuf[16];
	int subIdBufSize;

	char eventIdBuf[16];
	int eventIdBufSize;
}LogMgr;
extern LogMgr* g_plogMgr;

int SprintfBit(char buf[], const char* str, uint32 val, uint8* numberOfBit);

void Log_Init(LogMgr * logMgr, const LogCfg * cfg, const RecordCfg * recordCfg);
void Log_Write(LogMgr * logMgr, uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val);

void Log_DumpByMask(LogMgr * logMgr, const LogItem * dst, const LogItem * mask, int count);
void Log_DumpByCount(LogMgr * logMgr, int count);
void Log_DumpByInd(LogMgr * logMgr, int ind, int count);

void Log_Tester();
void Log_Dump(LogMgr* logMgr, LogItem* pItem, const char* head, const char* tail);

#else
#define LOG_TRACE1(...) 			
#define LOG_TRACE2(...)		
#define LOG_TRACE3(...)
#define LOG_TRACE4(...)

#define LOG_WARNING1(...)	
#define LOG_WARNING2(...)		
#define LOG_WARNING3(...)
#define LOG_WARNING4(...)

#define LOG_ERROR1(...)			
#define LOG_ERROR2(...)		
#define LOG_ERROR3(...)	
#define LOG_ERROR4(...)
#endif

#ifdef __cplusplus
}
#endif

#endif

