#ifndef __COMMON_H_
#define __COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#ifdef WIN32
#else
#define XDEBUG
#endif

#include "ArchDef.h"
#include "Obj.h"
#include "Utp.h"
#include "debug.h"

#ifdef __cplusplus
}
#endif

#endif

