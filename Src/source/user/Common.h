#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#ifdef WIN32
#else
#endif

//#define CFG_LOG
//#define USE_NFC_THREAD

#include "ArchDef.h"
#include "Obj.h"
#include "Utp.h"
#include "Bit.h"
#include "SwTimer.h"
#include "message.h"
#include "queue.h"
#include "datetime.h"
#include "Log.h"
#include "McuFlashMap.h"
#include "ExternFlashMap.h"
#include "NvdsUser.h"
#include "rtthread.h"
#include "gd32e10x.h"
#include "LogUser.h"
#include "SysLog.h"
#include "drv_pm.h"
#include "drv_gpio.h"
#include "Debug.h"
#include "pms.h"
#include "drv_rtc.h"
#include "main.h"
#include "HwFwVer.h"
#include "nvc.h"
#include "CmdLineExport.h"
#include "Dump.h"
#include "Ble.h"
#include "workmode.h"

#define CFG_CABIN_LOCK


#ifdef __cplusplus
}
#endif

#endif


