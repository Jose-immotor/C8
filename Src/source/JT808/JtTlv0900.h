
#ifndef  _JT_TLV_0900_H_
#define  _JT_TLV_0900_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "BmsReg.h"

	typedef enum
	{
		TAG_BAT_ID = 1,
		TAG_BAT_INFO = 2,
	}CMD0900_TLV;

	typedef struct _TlvBatInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 port;
		uint16 nominalVol;
		uint16 nominalCur;
	}TlvBatInfo;

	typedef struct _TlvBatWorkInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 soc;
		uint16 voltage;
		int16 current;
		uint16 maxCellVol;
		uint16 minCellVol;
		uint8 maxCellNum;
		uint8 minCellNum;

		uint16 maxChgCurr;
		uint16 maxDischgCurr;
		uint16 curWorkFeature;

		uint16 cycCount;
	}TlvBatWorkInfo;

	void JtTlv0900_init();
	int JtTlv0900_getChangedTlv(uint8* buf, int len);
	void JtTlv0900_updateMirror(const uint8* data, int len);
	void JtTlv0900_updateBatInfo(uint8 port);

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


