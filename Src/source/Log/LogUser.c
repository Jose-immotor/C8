
#include "Common.h"
#include "LogUser.h"
#include "SysLog.h"

LogMgr* g_plogMgr;

//��Flsah��ȡ����
Bool LogItem_FmcRead(uint32 addr, void* buf, int len)
{
	return False;
}

//д���ݵ�Flsah
Bool LogItem_FmcWrite(uint32 addr, const void* pData, int len)
{
	return False;
}

//Flsah����
void LogItem_FmcErase(uint32 addr, int len)
{
}

Bool LogItem_Verify(const LogItem* pItem)
{
	return True;
}

void LogUser_init()
{
	static LogMgr g_logMgr;
	g_plogMgr = &g_logMgr;

	static const LogModule g_logModules[] =
	{
		{LogModuleID_SYS, "system", g_SysLogEvts, SYS_LOG_EVT_COUNT},
	};

	static const LogCfg cfg =
	{
		.logVersion = 1,
		.moduleArray = g_logModules,
		.moduleCount = GET_ELEMENT_COUNT(g_logModules),
		.Event = Null,
		.GetCurSec = Null,
	};

	static LogItem g_logItem;
	static LogItem g_logItem_exchangeBuf;

	static const RecordCfg recordCfg = 
	{
		.base.startAddr = LOG_AREA_ADDR,	//����������ʵ�ʷ���
		.base.sectorSize = EX_FLASH_SECTOR_SIZE,	//����������ʵ�ʶ���
		.base.storage = &g_logItem,
		.base.storageSize = sizeof(LogItem),
		.base.exchangeBuf = &g_logItem_exchangeBuf,
		.base.exchangeBufSize = sizeof(g_logItem_exchangeBuf),

		.base.Verify = (ItemVerifyFn)LogItem_Verify,
		.base.Read  = LogItem_FmcRead,	
		.base.Write = LogItem_FmcWrite,	
		.base.Erase = LogItem_FmcErase,	

		.sectorCount = 2,	//����������ʵ�ʷ���
	};

	Log_Init(&g_logMgr, &cfg, &recordCfg);
}
