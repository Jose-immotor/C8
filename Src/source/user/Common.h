#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#ifdef WIN32
#else
//#define XDEBUG
#endif

#define CFG_LOG

#include "ArchDef.h"
#include "Obj.h"
#include "Utp.h"
#include "Bit.h"
#include "SwTimer.h"
#include "message.h"
#include "queue.h"
#include "datatime.h"
#include "Log.h"
#include "McuFlashMap.h"
#include "ExternFlashMap.h"
#include "NvdsUser.h"
#include "rtthread.h"
#include "gd32f403.h"
#include "LogUser.h"

#ifdef __cplusplus
}
#endif

#endif


