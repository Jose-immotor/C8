#ifndef __HW_FW_VER_H_
#define __HW_FW_VER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#pragma pack(1) 
	typedef struct _HwFwVer
	{
		uint8_t  m_AppMainVer;
		uint8_t  m_AppSubVer;
		uint8_t  m_AppMinorVer;
		uint32_t m_AppBuildeNum;

		uint8_t m_HwMainVer;
		uint8_t m_HwSubVer;
	}HwFwVer;
#pragma pack() 

	void HwFwVer_Dump(const char* headStr, const HwFwVer* pHwFwVer, const char* tailStr);
	Bool HwFwVer_FwVerIsEqual(const HwFwVer* pOld, const HwFwVer* pNew);
	Bool HwFwVer_HwVerIsEqual(const HwFwVer* pOld, const HwFwVer* pNew);
	Bool HwFwVer_FwIsValid(const HwFwVer* pVer);
	Bool HwFwVer_HwIsValid(const HwFwVer* pVer);

#ifdef __cplusplus
}
#endif

#endif

