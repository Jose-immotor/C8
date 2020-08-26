
#include "ArchDef.h"
#include "Log.h"
#include "Record.h"
#include "DateTime.h"

#ifdef CFG_LOG

LogMgr* g_plogMgr;

void Log_Dump(LogMgr* logMgr, LogItem* pItem, const char* head, const char* tail);


/*******************************
�������ܣ��������ַ�������combo string��������braceCheck("0-title1|1-title2", 1, buf, 0) => "0:title1"
����˵����
	string����������ַ�������ʽΪ "0-title1|1-title2"
	value:  �ַ�����ֵ��
	buf:    ���������
	offset: ����������������ݵ�ƫ��ֵ
����ֵ��buf��������������֮���ƫ��ֵ
***********************************/
int braceProc(const char* str, int value, char* buf, int offset)
{
	Bool isMatch = False;
	int comboStrVal = 0;
	while (str)
	{
		int ret = sscanf(str, "%d-", &comboStrVal);
		if (sscanf(str, "%d-", &comboStrVal) == 1 && comboStrVal == value)
		{
			//����":"֮ǰ����ֵ
			//str = strchr(str, ':') + 1;
			while ((*str != '|') && (*str != 0))
			{
				isMatch = True;
				buf[offset++] = *str;
				str++;
			}
			break;
		}
		str = strchr(str, '|');
		if (str) str += 1;	//Խ���ַ�'|'
	}

	if (!isMatch)
	{
		offset += sprintf(&buf[offset], "%d-unknown", value);
	}

	return offset;
}

/************************************************************
�������ܣ�32λ��������BIT��ʽ�������
����˵����
	buf�������������
	val�������Ŀ��ֵ��
	fmt����ʽ�����ݣ�֧�����¸�ʽ��
		"%1B"		//���val.BIT[0], ��ֵ��ʾΪ10����
		"%1BX"		//���������һ��
		"%0-1B"		//���val.BIT[0-1], ��ֵ��ʾΪ10����
		"%2-6BX"	//���val.BIT[2-6], ��ֵ��ʾΪ16����
		"%7-10B{1:Title1|2:Title2|...}"//���val.BIT[7-10]����ֵʹ�ô������ڵ��ַ�������,�����ŵ��ַ������Ȳ��ܳ���128���ֽڡ�
		"%7-10BX{1:Title1|2:Title2|...}"//���������һ����
	numberOfBit�����������ʽfmt�����val����ЧBit���������ΪNull����ʾ�������������
����ֵ��buf�ĳ���
************************************************************/
int SprintfBit(char buf[], const char* fmt, uint32 val, uint8* numberOfBit)
{
	Bool isMatch = False;
	uint8 byte, byte1, byte2 = 0;
	uint8 bits = 0;
	int v1, v2, len = 0;
	const char* s = fmt;
	const char* p = strstr(fmt, "%");
	char brace[128];

	while (p)
	{
		static int8 strFmt[] = { '%', 'X', 0 };
		memcpy(&buf[len], s, (uint32)p - (uint32)s);
		len += (uint32)p - (uint32)s;
		s = p;

		p++;

		if ((3 <= sscanf(p, "%d%c%c%c", &v1, &byte, &byte1, &byte2)) && (byte == 'B' || byte == 'b'))	//Search string "%0B" or "%0BX"
		{
			isMatch = True;

			//��ȡ�ַ�����{}֮����ַ� x#${0:title1|1:title2}=>0:title1|1:title2"
			if ((byte1 == '{' || byte2 == '{') && sscanf(p, "%*[^{]{%[^}]", brace) == 1)
			{
				//�������ڵ�COMBO�ַ�����
				len = braceProc(brace, GET_BITS(val, v1, v1), buf, len);
				s = strstr(p, "}") + 1;
			}
			else
			{
				len += sprintf(&buf[len], "%d", GET_BITS(val, v1, v1));
				if (byte1 == 'X' || byte1 == 'x')
					s = strchr(p, byte1) + 1;
				else
					s = strstr(p, "B") + 1;
			}

			bits = MAX(bits, v1);
		}
		else
		{
			int ret = sscanf(p, "%d-%d%c%c%c", &v1, &v2, &byte, &byte1, &byte2);

			if (ret >= 3 && (byte == 'B' || byte == 'b')) //%1-7B
			{
				isMatch = True;

				v1 = MIN(v1, v2);
				v2 = MAX(v1, v2);

				if (ret >= 4)	//%1-7BX
				{
					strFmt[1] = (byte1 == 'X' || byte1 == 'x') ? byte1 : 'd';
				}
				if ((byte1 == '{' || byte2 == '{') && sscanf(p, "%*[^{]{%[^}]", brace) == 1)	//%1-7BX{...}
				{
					//�������ڵ�COMBO�ַ�����
					len = braceProc(brace, GET_BITS(val, v1, v2), buf, len);
					s = strstr(p, "}") + 1;
				}
				else
				{
					len += sprintf(&buf[len], strFmt, GET_BITS(val, v1, v2));
					if (byte1 == 'X' || byte1 == 'x')
						s = strchr(p, byte1) + 1;
					else
						s = strstr(p, "B") + 1;
				}
				bits = MAX(bits, v1);
				bits = MAX(bits, v2);
			}
		}

		//Search next "%..B"
		p = strstr(p, "%");
	}

	//Copy tail of string to buff
	p = (char*)(fmt + strlen(fmt));
	memcpy(&buf[len], s, (uint32)p - (uint32)s);
	len += (uint32)p - (uint32)s;
	buf[len++] = 0;

	if (numberOfBit) * numberOfBit = bits;
	return isMatch ? len : 0;
}

/************************************
�������ܣ���������������ӡ��־
����˵����
	dst���Ƚ�������
	mask���Ƚ�����, mask�г�Ա����Ϊ1�Ĳ�����Ҫ��Ϊ�Ƚ�����
����ֵ���ޡ�
************************************/
void Log_DumpByMask(LogMgr* logMgr, const LogItem* dst, const LogItem* mask, int count)
{
	int i = 0;
	LogItem item = { 0 };
	Record* pRecord = &logMgr->record;

	Record_Seek(pRecord, 0);

	for (i = 0; i < (int)pRecord->itemCount || count == 0; i++)
	{
		if (Record_Read(pRecord, &item, sizeof(LogItem)))
		{
			//���dateTime�ǱȽ�������С��Ŀ��ʱ��ֵdateTime����Ϊ����������
			if ((mask->dateTime == 0xFFFFFFFF) && (dst->dateTime < item.dateTime))
			{
				continue;
			}
			if ((mask->logType == 3) && (dst->logType != item.logType))
			{
				continue;
			}
			if ((mask->catId == 7) && (dst->catId != item.catId))
			{
				continue;
			}
			if ((mask->moduleId == 0xFF) && (dst->moduleId != item.moduleId))
			{
				continue;
			}
			if ((mask->subId == 0xFF) && (dst->subId != item.subId))
			{
				continue;
			}
			if ((mask->eventId == 0xFF) && (dst->eventId != item.eventId))
			{
				continue;
			}
			if ((mask->asUint32 == 0xFFFFFFFF) && (dst->asUint32 != item.asUint32))
			{
				continue;
			}
			count--;
			Log_Dump(logMgr, &item, Null, Null);
		}
	}
}
const char* DateTime_FmtStr(uint32 seconds)
{
	//2019-12-16 11:32:20
	static char timeStr[20];
	struct tm* pTm = localtime((time_t*)& seconds);
	strftime(timeStr, 20, "%Y-%m-%d %H:%M:%S", pTm);
	//remove year
	return &timeStr[5];
}

const char* LogType_ToStr(LogMgr* logMgr, const LogItem* pItem)
{
	if (pItem->logType == LT_WARNING) return "WARNING ";
	if (pItem->logType == LT_ERROR) return "ERROR ";
	return "";
}

static const LogModule * Get_logModule(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* p = logMgr->cfg->moduleArray;
	for (int i = 0; i < logMgr->cfg->moduleCount; i++, p++)
	{
		if (pItem->moduleId == p->moduleId)
		{
			return p;
		}
	}
	return Null;
}

void Log_DumpByCount(LogMgr * logMgr, int count)
{
	int ind = logMgr->record.itemCount - count;
	if (ind < 0) ind = 0;

	Log_DumpByInd(logMgr, ind, count);
}

void Log_DumpByInd(LogMgr * logMgr, int ind, int count)
{
	LogItem item = { 0 };

	Record_Seek(&logMgr->record, ind);

	while (Record_isValid(&logMgr->record) && count--)
	{
		if (Record_Read(&logMgr->record, &item, sizeof(LogItem)))
		{
			Log_Dump(logMgr, &item, Null, Null);
		}
	}
}

void Log_DumpByTime(LogMgr * logMgr, uint32 seconds, int count)
{
	LogItem mask, dst;
	memset(&mask, 0, sizeof(mask));
	mask.dateTime = 0xFFFFFFFF;

	dst.dateTime = seconds;
	Log_DumpByMask(logMgr, &dst, &mask, count);
}


//2008/04/03 11:12:00
void Log_DumpByTimeStr(LogMgr * logMgr, const char* pDataTime, int count)
{
	int year, month, day, hour, minute, sec;
	if (6 == sscanf(pDataTime, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &minute, &sec))
	{
		uint32 seconds = (uint32)DateTime_GetSeconds(DateTime_MakeRtc(year, month, day, hour, minute, sec));
		Log_DumpByTime(logMgr, seconds, count);
	}
	else
	{
		Printf("Date time format error\n");
	}
}

Bool Log_Event(LogMgr * logMgr, const LogItem * pItem, LOG_EVENT ev)
{
	if (logMgr->cfg->Event)
		return logMgr->cfg->Event(logMgr, pItem, ev);

	return True;
}

const LogEvt* Log_GetLogEvt(const LogEvt * pLogEvt, int count, const LogItem * pItem)
{
	for (int i = 0; i < count; i++, pLogEvt++)
	{
		if (pLogEvt->catId == pItem->catId
			&& pLogEvt->subId == pItem->subId
			&& pLogEvt->eventId == pItem->eventId)
		{
			return pLogEvt;
		}
	}

	return Null;
}

const char* EventId_ToStr(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* pModule = Get_logModule(logMgr, pItem);

	if (pModule && pModule->logEvtArray)
	{
		const LogEvt* pLogEvt = Log_GetLogEvt(pModule->logEvtArray, pModule->logEvtArrayCount, pItem);
		if (pLogEvt)
		{
			return pLogEvt->pEventIdStr;
		}
	}

	sprintf(logMgr->eventIdBuf, "EventID[%02X]", pItem->eventId);

	return logMgr->eventIdBuf;
}

const char* ModuleId_ToStr(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* p = Get_logModule(logMgr, pItem);
	return p->name;
}

const char* CatId_ToStr(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* p = Get_logModule(logMgr, pItem);
	sprintf(logMgr->catIdBuf, "%02X", pItem->eventId);
	return logMgr->catIdBuf;
}

const char* SubId_ToStr(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* p = Get_logModule(logMgr, pItem);
	sprintf(logMgr->subIdBuf, "%02X", pItem->eventId);
	return logMgr->subIdBuf;
}

const char* EventValue_ToStr(LogMgr * logMgr, const LogItem * pItem, const char* fmt)
{
	uint8 bites;
	char sprintfBitbuf[LOG_STR_MAX_SIZE];
	if (fmt == Null) fmt = "0x%08X (%d)";
	const uint32 * pAsUint32 = (uint32*)pItem->data;

	//sample: fmt = "BOM ID[%0-7BX], FAULT[%8-15B]"
	if (SprintfBit(sprintfBitbuf, fmt, *pAsUint32, &bites))
	{
		if (strlen(sprintfBitbuf) > LOG_STR_MAX_SIZE)
		{
			sprintf(logMgr->eventValueBuf, "* length[%d] error *", strlen(sprintfBitbuf));
		}
		else
		{
			if (bites > 24)      fmt = "0x%08X (%s)";
			else if (bites > 16) fmt = "0x%06X (%s)";
			else if (bites > 8)  fmt = "0x%04X (%s)";
			else                 fmt = "0x%02X (%s)";
			sprintf(logMgr->eventValueBuf, fmt, *pAsUint32, sprintfBitbuf);
		}
	}
	else
	{
		sprintf(logMgr->eventValueBuf, fmt, *pAsUint32, *pAsUint32);
	}
	return logMgr->eventValueBuf;
}

const char* LogVal_ToStr(LogMgr * logMgr, const LogItem * pItem)
{
	const LogModule* pModule = Get_logModule(logMgr, pItem);
	const LogEvt* pLogEvt = Null;

	if (pModule && pModule->logEvtArray)
	{
		pLogEvt = Log_GetLogEvt(pModule->logEvtArray, pModule->logEvtArrayCount, pItem);
	}

	return EventValue_ToStr(logMgr, pItem, pLogEvt ? pLogEvt->pEventValueStrFmt : Null);
}

void Log_Write(LogMgr * logMgr, uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val)
{
	LogItem* p = logMgr->recordCfg->base.storage;

	p->dateTime = logMgr->cfg->GetCurSec ? logMgr->cfg->GetCurSec() : 0;
	p->moduleId = moduleId;
	p->subId = subId;
	p->logType = logType;
	p->catId = catId;

	p->eventId = eventId;
	memcpy(p->data, &val, 4);

	Log_Event(logMgr, p, LOG_WRITE_BEFORE);

	Record_Write(&logMgr->record, p);

	Log_Event(logMgr, p, LOG_WRITE_AFTER);
}

void Log_Dump(LogMgr * logMgr, LogItem * pItem, const char* head, const char* tail)
{
//	const LogCfg* cfg = logMgr->cfg;

	if (head == Null) head = "";
	if (tail == Null) tail = "";

	memset(logMgr->catIdBuf		, 0, logMgr->catIdBufSize);
	memset(logMgr->subIdBuf		, 0, logMgr->subIdBufSize);
	memset(logMgr->eventIdBuf	, 0, logMgr->eventIdBufSize);
	memset(logMgr->eventValueBuf, 0, logMgr->eventValueBufSize);

	Printf("%s", head);
	Printf("%s:", DateTime_FmtStr(pItem->dateTime));
	Printf("%s %s.%s.%s.%s=%s%s\n"
		, LogType_ToStr(logMgr, pItem)
		, ModuleId_ToStr(logMgr, pItem)
		, CatId_ToStr(logMgr, pItem)
		, SubId_ToStr(logMgr, pItem)
		, EventId_ToStr(logMgr, pItem)
		, LogVal_ToStr(logMgr, pItem)
		, tail
	);
}

//����LOG�������Ƿ��ʱ������汾�Ų�һ�£�˵�����ݹ�ʱ���ᱻ����
Bool Log_Verify(LogMgr * logMgr, const LogItem * pItem, uint32 len)
{
	return (pItem->version == logMgr->cfg->logVersion);
}

void Log_Init(LogMgr * logMgr, const LogCfg * cfg, const RecordCfg * recordCfg)
{
	memset(logMgr, 0, sizeof(LogMgr));
	g_plogMgr = logMgr;

	logMgr->cfg = cfg;
	logMgr->recordCfg = recordCfg;

	logMgr->catIdBufSize = sizeof(logMgr->catIdBuf);
	logMgr->subIdBufSize = sizeof(logMgr->catIdBuf);
	logMgr->eventIdBufSize = sizeof(logMgr->catIdBuf);

	Record_Init(&logMgr->record, recordCfg);

	//����logEvt��pEventValueStrFmt�����Ƿ�Ϸ�
	const LogModule* logMod = cfg->moduleArray;
	for (int i = 0; i < cfg->moduleCount; i++, logMod++)
	{
		const LogEvt* logEvt = logMod->logEvtArray;
		if (logEvt)
		{
			for (int j = 0; j < logMod->logEvtArrayCount; j++, logEvt++)
			{
				if (strlen(logEvt->pEventValueStrFmt) > LOG_STR_MAX_SIZE)
				{
					Printf("LogEvt->EventValueStrFmt length error, must be < %d: %s", LOG_STR_MAX_SIZE, logEvt->pEventValueStrFmt);
					Assert(False);
				}
			}
		}
	}
}

//���´��������Logģ�����
#if 0
void Log_TestInit(int sectorCount)
{
	static LogItem	g_LogItem;
	g_LogItem.version = LOG_VERSION;
	g_LogItem.subId = LOG_LATEST_BYTE;

	//��ʼ����־��¼ģ��
	g_pLogItem = &g_LogItem;
	Record_Init(&logMgr->record
		, BANK1_SECTOR_SIZE
		, BANK1_PAGE_SIZE
		, 2
		, (void*)g_pMemMap->logArea
		, g_pLogItem
		, sizeof(LogItem)
		, (RecordVerifyItemFn)Log_Verify);
}

//����2������
void Log_TesterFor2Sec()
{
	int i = 0;
	int size = 4096 / sizeof(LogItem);
	Record_RemoveAll(&logMgr->record);

	Log_TestInit(2);

	//��0������дlogMgr->record.itemsPerSec - 1��Ԫ��
	size = logMgr->record.itemsPerSec - 1;
	for (i = 0; i < size; i++)
	{
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readPointer == 0);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(2);
	Assert(logMgr->record.readPointer == 0);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	//��0������д1��Ԫ�أ���0������
	Log_TestInit(2);
	Log_Write(1, 1, 1, i++);
	size++;
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(2);
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	//��1������дlogMgr->record.itemsPerSec��Ԫ�أ���
	size += logMgr->record.itemsPerSec;
	for (; i < size; i++)
	{
		Log_TestInit(2);
		Log_Write(1, 1, 1, i);
	}

	size -= logMgr->record.itemsPerSec;
	i -= logMgr->record.itemsPerSec;
	Assert(logMgr->record.readStartSectorInd == 1);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	Printf("Log test passed\n.");
}

//����3������
void Log_TestFor3Sect()
{
	Record_RemoveAll(&logMgr->record);

	Log_TestInit(3);
	int i = 0;
	int size = 4096 / sizeof(LogItem);
	Assert(logMgr->record.itemsPerSec == size);

	//��0������дlogMgr->record.itemsPerSec - 1��Ԫ��
	size = logMgr->record.itemsPerSec - 1;
	for (i = 0; i < size; i++)
	{
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readPointer == 0);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(3);
	Assert(logMgr->record.readPointer == 0);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	//��0������д1��Ԫ�أ���0������
	Log_TestInit(3);
	Log_Write(1, 1, 1, i++);
	size++;
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(3);
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	//��1������дlogMgr->record.itemsPerSec��Ԫ�أ���
	size += logMgr->record.itemsPerSec;
	for (; i < size; i++)
	{
		Log_TestInit(3);
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 2);
	Assert(logMgr->record.totalCount == size);

	//��2������дlogMgr->record.itemsPerSec-1��Ԫ��
	Log_TestInit(3);
	size += logMgr->record.itemsPerSec - 1;
	for (; i < size; i++)
	{
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 2);
	Assert(logMgr->record.totalCount == size);

	//��2������д1һ��Ԫ�أ���2������
	Log_Write(1, 1, 1, i++);
	size++;

	size -= logMgr->record.itemsPerSec;
	i -= logMgr->record.itemsPerSec;
	Assert(logMgr->record.readStartSectorInd == 1);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(3);
	Assert(logMgr->record.readStartSectorInd == 1);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	//��0������дlogMgr->record.itemsPerSec - 1��Ԫ��
	size += logMgr->record.itemsPerSec - 1;
	for (; i < size; i++)
	{
		Log_TestInit(3);
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readStartSectorInd == 1);
	Assert(logMgr->record.writeSectorInd == 0);
	Assert(logMgr->record.totalCount == size);

	//��0������д1��Ԫ�أ���
	Log_TestInit(3);
	Log_Write(1, 1, 1, i++);
	size++;

	size -= logMgr->record.itemsPerSec;
	i -= logMgr->record.itemsPerSec;
	Assert(logMgr->record.readStartSectorInd == 2);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(3);
	Assert(logMgr->record.readStartSectorInd == 2);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	//��1������дlogMgr->record.itemsPerSec-1Ԫ��
	size += logMgr->record.itemsPerSec - 1;
	for (; i < size; i++)
	{
		Log_TestInit(3);
		Log_Write(1, 1, 1, i);
	}
	Assert(logMgr->record.readStartSectorInd == 2);
	Assert(logMgr->record.writeSectorInd == 1);
	Assert(logMgr->record.totalCount == size);

	//��1������д1��Ԫ�أ���
	Log_TestInit(3);
	Log_Write(1, 1, 1, i++);
	size++;
	size -= logMgr->record.itemsPerSec;
	i -= logMgr->record.itemsPerSec;
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 2);
	Assert(logMgr->record.totalCount == size);

	Log_TestInit(3);
	Assert(logMgr->record.readStartSectorInd == 0);
	Assert(logMgr->record.writeSectorInd == 2);
	Assert(logMgr->record.totalCount == size);

	Printf("Log test passed\n.");
}

#endif


#endif

