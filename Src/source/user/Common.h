#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#ifdef WIN32
#else
#endif

//#define CFG_LOG
#define USE_NFC_THREAD

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
#include "gd32f403.h"
#include "LogUser.h"
#include "SysLog.h"
#include "drv_pm.h"
#include "Debug.h"
	
#ifdef __cplusplus
}
#endif

#endif


