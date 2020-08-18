/*
 * File      : datarom.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-18     lane      first implementation
 */
#include "Common.h"
#include "datarom.h"
//#include "crc32.h"
//#include "drv_flash.h"

const VerDesc g_DefaultVerDesc = {5,0,1,0,0,2};


//static uint32 g_DataFlashAddr = 0x080ff800;//0xfc00;1023页 Bank2

DataRom* g_pDataRom = Null;		//Data Rom address.

void VerDesc_Dump(const VerDesc* pVerDesc, const char* pTag)
{
	Printf("%sHw(%d.%d), Fw(%d.%d.%d.%d)\n"
		, pTag
		, pVerDesc->m_HwMainVer
		, pVerDesc->m_HwSubVer
		
		, pVerDesc->m_AppMainVer
		, pVerDesc->m_AppSubVer
		, pVerDesc->m_AppMinorVer
		, pVerDesc->m_AppBuildeNum);
}

void DaraRom_Dump()
{
	if(g_pDataRom->m_isUpdateFw)
	{
		g_pDataRom->m_AppMainVer 	= g_pDataRom->m_OlderAppMainVer ;
		g_pDataRom->m_AppSubVer 	= g_pDataRom->m_OlderAppSubVer 	;
		g_pDataRom->m_AppMinorVer 	= g_pDataRom->m_OlderAppMinorVer ;
		g_pDataRom->m_AppBuildeNum  = g_pDataRom->m_OlderAppBuildeNum;
	}
 	Printf("\nHw Version:%d.%d\n", g_pDataRom->m_HwMainVer, g_pDataRom->m_HwSubVer);
	Printf("Fw Version:%d.%d.%d.%d\n"
		, g_pDataRom->m_AppMainVer
		, g_pDataRom->m_AppSubVer
		, g_pDataRom->m_AppMinorVer
		, g_pDataRom->m_AppBuildeNum);
	Printf("IpAddr:%s, Port=%d.\n", g_pDataRom->m_IpAddr, g_pDataRom->m_IpPort);
}

void DaraRom_Init()
{
	uint32 crc = 0;
	uint32 romCrc = 0;
	const DataRom* pConstDataRom = Null;
	static DataRom g_DataRom__;
	uint8* pByte = 0;
	
	g_pDataRom = &g_DataRom__;
	
	//g_DataFlashAddr = FMC_ReadDataFlashBaseAddr();
//	pByte = (uint8*)(g_DataFlashAddr);
//	pConstDataRom = (DataRom*)g_DataFlashAddr;
	
//	crc = Crc32_Calc(CRC32_CHECK_CODE, (uint8*)g_DataFlashAddr, sizeof(DataRom));
//	romCrc = AS_UINT32(pByte[3],pByte[2],pByte[1],pByte[0]);
//	if(crc == romCrc)
//	{
//		memcpy(g_pDataRom, pConstDataRom, sizeof(DataRom));
//	}
//	else
//	{
		g_pDataRom->m_Version = 3;	//支持从DataRom中获取IP地址和端口号。
		
		g_pDataRom->m_HwMainVer = g_DefaultVerDesc.m_HwMainVer;
		g_pDataRom->m_HwSubVer  = g_DefaultVerDesc.m_HwSubVer;
		g_pDataRom->m_AppMainVer = g_DefaultVerDesc.m_AppMainVer;
		g_pDataRom->m_AppSubVer  = g_DefaultVerDesc.m_AppSubVer;
		g_pDataRom->m_AppMinorVer = g_DefaultVerDesc.m_AppMinorVer;
		g_pDataRom->m_AppBuildeNum  = g_DefaultVerDesc.m_AppBuildeNum;
		strcpy(g_pDataRom->m_IpAddr, IP_ADDR);
		g_pDataRom->m_IpPort = IP_PORT;
//	}
	DaraRom_Dump();
}

void DaraRom_SetIp(char* ipAddr, uint16 port)
{
	memset(g_pDataRom->m_IpAddr, 0, sizeof(g_pDataRom->m_IpAddr));
	strcpy(g_pDataRom->m_IpAddr, ipAddr);
	g_pDataRom->m_IpPort = port;
	DataRom_Write();

	Printf("Set IpAddr:%s,Port:%d success.\n", g_pDataRom->m_IpAddr, port);
}

void DaraRom_WriteFlash(uint32 addr, const void* pData, int len)
{
	int i = 0;
	const uint8* pByte = (uint8*)pData;
//	uint32 value = 0;

	for(i = 0; i < len; i++, addr += 4, pByte += 4)
	{
//		value = AS_UINT32(pByte[3], pByte[2], pByte[1], pByte[0]);
//		at32_flash_write(addr, pByte, 1);
//		FMC_Write(addr, value);
	}
}

void DataRom_SetUpdateFwFlag(Bool bFlag)
{
	g_pDataRom->m_isUpdateFw = bFlag;
}

Bool DataRom_IsFactoryMode()
{
	return g_pDataRom->m_isFactoryMode;
}

void DataRom_WriteFactoryMode(uint8 mode)
{
	g_pDataRom->m_isFactoryMode = mode;
	DataRom_Write();
}

void DataRom_Write()
{
	uint32 crc = 0;
	
//	memset(g_CommonBuf, 0, sizeof(g_CommonBuf));
//	memcpy(g_CommonBuf, g_pDataRom, sizeof(DataRom));
//			
////	crc = Crc32_Calc(CRC32_CHECK_CODE, g_CommonBuf, sizeof(DataRom));
//	memcpy(&g_CommonBuf[508], &crc, sizeof(crc));

//	FMC_Erase(g_DataFlashAddr);
//	at32_flash_erase(g_DataFlashAddr,512);
	//Printf("DataRom write, isUpdateFw=%d\n", g_pDataRom->m_isUpdateFw);
//	DaraRom_WriteFlash(g_DataFlashAddr, g_CommonBuf, 512);
//	at32_flash_write(g_DataFlashAddr, g_CommonBuf, 512);
}

void DaraRom_Test(void)
{
	uint8_t data[4]; 
//	uint32 addr = g_DataFlashAddr;
	
//	at32_flash_read(addr,data,sizeof(data));
//	Printf("read data=0x%x,0x%x,0x%x,0x%x\n", data[0],data[1],data[2],data[3]);
//	
//	at32_flash_erase(addr,sizeof(data));
//	Printf("Erase data\n");

//	at32_flash_read(addr,data,sizeof(data));
	Printf("read value=0x%x,0x%x,0x%x,0x%x\n", data[0],data[1],data[2],data[3]);

	data[0]=0x12;
	data[1]=0x23;
	data[2]=0x34;
	data[3]=0x56;
//	at32_flash_write(addr,data,sizeof(data));
//	Printf("write value=0x%x,0x%x,0x%x,0x%x\n", data[0],data[1],data[2],data[3]);
//	at32_flash_read(addr,data,sizeof(data));
	Printf("read value=0x%x,0x%x,0x%x,0x%x\n", data[0],data[1],data[2],data[3]);
	
}
