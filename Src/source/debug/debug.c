
#include "Common.h"
#include "Debug.h"

#ifdef XDEBUG

//���屾ϵͳ�ĵ��Կ��ر�
static const DbgSwitch g_defDbgSwh[] =
{
	DBG_LEV_DEF(DL_CAN),
	DBG_LEV_DEF(DL_IO),
	DBG_LEV_DEF(DL_ADC),

	DBG_LEV_DEF(DL_JT808),
};

void Debug_Init()
{
	Dbg_Init(Null, g_defDbgSwh, GET_ELEMENT_COUNT(g_defDbgSwh));
}

#endif

