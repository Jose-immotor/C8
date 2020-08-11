/*
 * File      : debug.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */

#include "common.h"
#include "ArchDef.h"
#include "Debug.h"
//#include "Nvds.h"

#ifdef XDEBUG
unsigned int g_dwDebugLevel = 0;

void Debug_Init(void)
{
	g_dwDebugLevel = 0;
/*	

	g_dwDebugLevel |= DL_ERROR;
	g_dwDebugLevel |= DL_WARNING;
	g_dwDebugLevel |= DL_ASSERT;	
	
	g_dwDebugLevel |= DL_BLE;
	g_dwDebugLevel |= DL_GPS;
	g_dwDebugLevel |= DL_GPRS;	
	g_dwDebugLevel |= DL_ATCCMD;	

	*/
	
}

/*!
 * \brief 设置Debug输出等级
 *		  
 * \param  
 *
 * \return NONE 
 */
static void Debug_SetL(int argc, char**argv)
{
	int level;
	
	sscanf(&(*argv[1]), "%d", &level);
	if(level == 0)
	{
		g_dwDebugLevel = 0;
	}
	else 
	{
		g_dwDebugLevel = DL_MAIN;
		g_dwDebugLevel |= DL_ERROR;
		g_dwDebugLevel |= DL_WARNING;
		g_dwDebugLevel |= DL_ASSERT;
		if(level > 1)
		{
			g_dwDebugLevel |= DL_BLE;
			g_dwDebugLevel |= DL_GPS;
			g_dwDebugLevel |= DL_GPRS;	
			g_dwDebugLevel |= DL_ATCCMD;	
			g_dwDebugLevel |= DL_SIM;	
			g_dwDebugLevel |= DL_FSM;	
			g_dwDebugLevel |= DL_LOG;	
		}
		if(level > 2)
		{
			g_dwDebugLevel = DL_MAIN;
			g_dwDebugLevel |= DL_GYRO;	
			g_dwDebugLevel |= DL_FSM;	
		}
		if(level > 3)
		{
			g_dwDebugLevel |= DL_PMS;	
		}
		if(level > 9)
		{
			g_dwDebugLevel = DL_DEBUG;
		}
		if(level == 10)
		{
			g_dwDebugLevel = DL_MAIN;
			g_dwDebugLevel |= DL_GYRO;	
			g_dwDebugLevel |= DL_FSM;	
		}
	}
	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);
//	Nvds_Write_SysCfg();
}
MSH_CMD_EXPORT(Debug_SetL, sample: Debug_SetL <uint8_t level>);

/*!
 * \brief 设置Debug输出等级
 *		  
 * \param  
 *
 * \return NONE 
 */
static void Debug_SetB(int argc, char**argv)
{
	#define PFOUT(value) Printf("g_dwDebugLevel.[BIT:%02d].%s\t = %d.\n", GetBi\
	        tInd(value), #value, (g_dwDebugLevel >> GetBitInd(value)) & 0x01)
	uint32_t nIndex;
	int isEnable;
	
	sscanf(&(*argv[1]), "%d", &nIndex);
	sscanf(&(*argv[2]), "%d", &isEnable);	
	if(isEnable)
	{
		g_dwDebugLevel |= (1 << nIndex);
	}
	else
	{
		g_dwDebugLevel &= ~(1 << nIndex);
	}
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
	
	Printf("g_dwDebugLevel = 0x%08x\n", g_dwDebugLevel);	
//	Nvds_Write_SysCfg();
}
MSH_CMD_EXPORT(Debug_SetB, sample: Debug_SetB <nIndex isEnable>);

void Debug_DumpByteEx(uint32_t level, const uint8_t* pData, uint16_t len,
                      uint8_t cols, const char* str, int line) 
{
    int i; 
    int counter = 0;	

	if(!(level & g_dwDebugLevel)) return;
	
	if(str && line)
	{
		Printf("%s,line[%d],Len[%d]:",str,line,len); 
	}
	else if(str)
	{
		Printf("%s,Len[%d]:",str,len); 
	}
	else if(line)
	{
		Printf("line[%d],Len[%d]:",line,len); 
	}
	
    while(counter < len) 
    { 
        for(i=0; i < cols; i++) 
        { 
            Printf("%02x ",*pData); 
            pData++; 
            if(++counter >= len) break;	
        } 
    } 
    Printf("\n"); 
}

void ShowWait(int ticksPerDot, int dotCountPerRow)
{
	static int i = 0;
	static int count = 0;

	if( (i++) % ticksPerDot == 0)
	{
		if(count++ >= dotCountPerRow)
		{
			count = 0;
			i = 0;
			Printf("\n\t");
		}
		else
		{
			Printf(".");
		}
	}
}

void Debug_DumpByte(const uint8_t* pData, uint16_t len, uint8_t cols) 
{
    int i; 
    int counter = 0;	
	//Printf("Dump Data, addr=[0x%04x], Len=%d\n", pData, len); 
    while(counter < len) 
    { 
        //Printf("[%04x]:",counter); 
        for(i=0; i < cols; i++) 
        { 
            Printf("%02X ",*pData); 
            pData++; 
            if(++counter >= len) break;	
        } 
        Printf("\n"); 
    } 
}

void Debug_DumpDword(const uint32_t* pData, uint16_t len, uint8_t cols) 
{
    int i; 
    int counter = 0;	
    while(counter < len) 
    { 
        Printf("[%04X]:",counter); 
        for(i=0; i < cols; i++) 
        { 
            Printf("%08X ",*pData); 
            pData++; 
			
            if(++counter >= len) break;	
        } 
        Printf("\n"); 
    } 
}

#endif

