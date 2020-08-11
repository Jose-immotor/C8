
#include "Common.h"
#include "HwFwVer.h"

void HwFwVer_Dump(const char* headStr, const HwFwVer* pHwFwVer, const char* tailStr)
{
	if (headStr == Null) headStr = "";
	if (tailStr == Null) tailStr = "";

	Printf("%sHw(%d.%d), Fw(%d.%d.%d.%d)%s"
		, headStr
		, pHwFwVer->m_HwMainVer
		, pHwFwVer->m_HwSubVer

		, pHwFwVer->m_AppMainVer
		, pHwFwVer->m_AppSubVer
		, pHwFwVer->m_AppMinorVer
		, pHwFwVer->m_AppBuildeNum
		, tailStr
	);
}

Bool HwFwVer_FwVerIsEqual(const HwFwVer* pOld, const HwFwVer* pNew)
{
	return (memcmp(&pOld->m_AppMainVer, &pNew->m_AppMainVer, 7) == 0);
}

Bool HwFwVer_HwVerIsEqual(const HwFwVer* pOld, const HwFwVer* pNew)
{
	return (pOld->m_HwMainVer == pNew->m_HwMainVer && pOld->m_HwSubVer == pNew->m_HwSubVer);
}

Bool HwFwVer_FwIsValid(const HwFwVer* pVer)
{
	const static HwFwVer validFwVer1 = { 0,0,0,0};
	const static HwFwVer validFwVer2 = { 0xFF,0xFF,0xFF, 0xFFFFFFFF};
	return !((memcmp(pVer, &validFwVer1, sizeof(HwFwVer) - 2) == 0) || (memcmp(pVer, &validFwVer2, sizeof(HwFwVer) - 2) == 0));
}

Bool HwFwVer_HwIsValid(const HwFwVer* pVer)
{
	return !((pVer->m_HwMainVer == 0 && pVer->m_HwSubVer == 0) || (pVer->m_HwMainVer != 0xFF && pVer->m_HwSubVer != 0xFF));
}
