
#include "Common.h"
#include "Debug.h"
//#include "Nvds.h"

unsigned int g_dwDebugLevel = 0;
#ifdef XDEBUG

void Debug_Init()
{
	g_dwDebugLevel = 0;
/*	
	g_dwDebugLevel = DL_MAIN;
	g_dwDebugLevel |= DL_ERROR;
	g_dwDebugLevel |= DL_WARNING;
	g_dwDebugLevel |= DL_ASSERT;	
	
	g_dwDebugLevel |= DL_BLE;
	g_dwDebugLevel |= DL_GPS;
	g_dwDebugLevel |= DL_GPRS;	
	g_dwDebugLevel |= DL_ATCCMD;	
	g_dwDebugLevel |= DL_SIM;
	*/
	
}

void Debug_SetLevel(uint8_t level)
{
	if(level == 0)
	{
		g_dwDebugLevel = 0;
	}
	else 
	{
		g_dwDebugLevel = DL_MAIN;
		g_dwDebugLevel |= DL_ERROR;
		g_dwDebugLevel |= DL_WARNING;
		if(level > 1)
		{
		}
		if(level > 2)
		{
		}
	}
	
	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);	
	//Nvds_Write(NVDS_DBG_INFO);
}
	
void Debug_SetBit(uint32_t nIndex, Bool isEnable)
{
	#define PFOUT(value) Printf("\tBIT[%02d].%s\t = %d.\n", Uint32_t_getMaskBit(value), #value, (g_dwDebugLevel >> Uint32_t_getMaskBit(value)) & 0x01)

	if(isEnable)
	{
		g_dwDebugLevel |= (1 << nIndex);
	}
	else
	{
		g_dwDebugLevel &= ~(1 << nIndex);
	}

	PFOUT(DL_MAIN);
	PFOUT(DL_ERROR	);
	PFOUT(DL_WARNING);

	PFOUT(DL_IO);
	PFOUT(DL_ADC);


	Printf("g_dwDebugLevel = ");	
	for(int i = 0; i < 32; i++)
	{
		Printf("%d", (g_dwDebugLevel >> i) & 0x01);
		if((i+1) % 4 == 0)
		{
			Printf(" ");
		}
	}
	Printf("\n");
	
	//Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);	
	//Nvds_Write(NVDS_DBG_INFO);
}


#endif

