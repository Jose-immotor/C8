
#include "Common.h"
#include "LogUser.h"
#include "SysLog.h"
#include "drv_spi.h"

LogMgr* g_plogMgr;

Bool LogUserEvent(struct _LogMgr* p, const LogItem * pItem, LOG_EVENT ev)
{
	if(ev ==LOG_WRITE_AFTER)
	{
		if (g_dwDebugLevel & DL_LOG)
		{
			Printf("%04d:", p->record.sector.itemCount);
			Log_Dump(p, (LogItem *)pItem, Null, Null);
		}
	}
	return True;
}

//��Flsah��ȡ����
Bool LogItem_FmcRead(uint32 addr, void* buf, int len)
{
	spi_flash_buffer_read(addr,buf,len);
	return True;
}

//д���ݵ�Flsah
Bool LogItem_FmcWrite(uint32 addr, const void* pData, int len)
{
	spi_flash_buffer_write(addr,(uint8_t*)pData,len);
	return True;
}

//Flsah����
void LogItem_FmcErase(uint32 addr, int len)
{
	spi_flash_sector_erase(addr);
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
		.Event = LogUserEvent,
		.GetCurSec = DateTime_GetSeconds,
	};

	static LogItem g_logItem;
	static LogItem g_logItem_exchangeBuf;

	static const RecordCfg recordCfg = 
	{
		.base.startAddr = EX_FLASH_LOG_AREA_ADDR,	//����������ʵ�ʷ���
		.base.sectorSize = EX_FLASH_SECTOR_SIZE,	//����������ʵ�ʶ���
		.base.storage = &g_logItem,
		.base.storageSize = sizeof(LogItem),
		.base.exchangeBuf = &g_logItem_exchangeBuf,
		.base.exchangeBufSize = sizeof(g_logItem_exchangeBuf),

		.base.Verify = (ItemVerifyFn)LogItem_Verify,
		.base.Read  = LogItem_FmcRead,	
		.base.Write = LogItem_FmcWrite,	
		.base.Erase = LogItem_FmcErase,	

		.sectorCount = 4,	//����������ʵ�ʷ���
	};

	Log_Init(&g_logMgr, &cfg, &recordCfg);
}
