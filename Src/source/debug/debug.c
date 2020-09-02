
#ifdef XDEBUG
#include "Common.h"
#include "Debug.h"

//定义本系统的调试开关表
static const DbgSwitch g_defDbgSwh[] =
{
	DBG_LEV_DEF(DL_CAN),
	DBG_LEV_DEF(DL_IO),
	DBG_LEV_DEF(DL_ADC),
	DBG_LEV_DEF(DL_LOG),
};

void Debug_Init()
{
	Dbg_Init(Null, g_defDbgSwh, GET_ELEMENT_COUNT(g_defDbgSwh));
}

#endif

