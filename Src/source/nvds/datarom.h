#ifndef __DATA_ROM_H_
#define __DATA_ROM_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define GET_COUNTRY() (g_pDataRom->m_AppSubVer)

//App file info area
#pragma pack(1) 
typedef struct _VerDesc
{
	uint8 m_HwMainVer;
	uint8 m_HwSubVer;
	
	uint8  m_AppMainVer;
	uint8  m_AppSubVer;
	uint8  m_AppMinorVer;
	uint32 m_AppBuildeNum;
}VerDesc;

typedef struct _DataRom
{
	//Only write once section, 64 bytes
	uint8 m_Version;	//DataArea version.
	uint8 m_HwMainVer;
	uint8 m_HwSubVer;
	uint8 m_isUpdateFw;
	char  m_IpAddr[20];	//"255.255.255.255"
	uint16  m_IpPort;	
	
	uint8  m_isFactoryMode:1;
	uint8  m_isSlience:1;	//=1时，处于静默状态,蜂鸣器或者喇叭都不会响,用于RTC定时复位系统，设备必须保持静默，不能被用户感知
	uint8  m_reserved0:6;
	uint8 m_reserved1[9];

	//Updated with app upgrade, 64 bytes
	uint32 m_AppFileLen;
	uint32 m_AppFileCrc;
	
	uint8  m_AppMainVer;
	uint8  m_AppSubVer;
	uint8  m_AppMinorVer;
	uint32 m_AppBuildeNum;
	
	#if 1
	uint8  m_OlderAppMainVer;
	uint8  m_OlderAppSubVer;
	uint8  m_OlderAppMinorVer;
	uint32 m_OlderAppBuildeNum;
	#endif
	
	uint8  m_reserved2[8];
}DataRom;
#pragma pack() 

extern DataRom* g_pDataRom;

void DaraRom_Init(void);
void DataRom_Write(void);
void DataRom_SetUpdateFwFlag(Bool bFlag);
void DaraRom_SetIp(char* ipAddr, uint16 port);
void DataRom_WriteFactoryMode(uint8 mode);
Bool DataRom_IsFactoryMode(void);
void VerDesc_Dump(const VerDesc* pVerDesc, const char* pTag);

extern void DaraRom_Dump(void);

#ifdef __cplusplus
}
#endif

#endif




