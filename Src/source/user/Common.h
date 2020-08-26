#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#ifdef WIN32
#else
#define XDEBUG
#endif

#define CFG_LOG

#include "ArchDef.h"
#include "Obj.h"
#include "Utp.h"
#include "Bit.h"
#include "SwTimer.h"
#include "Log.h"
#include "FlashMap.h"

#ifdef __cplusplus
}
#endif

#endif


