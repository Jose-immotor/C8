#ifndef _NFC_ISO_14443_H
#define _NFC_ISO_14443_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "fm175Drv.h"
	typedef enum _NFC_ISO_TYPE
	{
		NFC_ISO_TYPE_A = 0,	//ISO14443_A
		NFC_ISO_TYPE_B = 1,	//ISO14443_B
	}NFC_ISO_TYPE;

	Bool NfcIso14443_CardActivate(Fm175Drv* pDrv);
	Bool NfcIso14443_RATS(Fm175Drv* pDrv, unsigned char param);
	Bool NfcIso14443_ConfigISOType(Fm175Drv* pDrv, NFC_ISO_TYPE type);
	Bool NfcIso14443_SoftPowerdown(Fm175Drv* pDrv);

#ifdef __cplusplus
}
#endif

#endif

