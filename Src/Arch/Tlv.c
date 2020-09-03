/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"


void Tlv_dump(uint32 tag, uint8 tagLen, int len, const uint8* val, DType dt)
{
	struct
	{
		DType dt;
		const char* fmt;
		DtConvertFn Convert;
	}
	static const disp[] =
	{
		{DT_UINT8 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU8 },
		{DT_UINT16, "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_UINT32, "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},
		{DT_INT8  , "0x%02X(%d)", (DtConvertFn)Dt_convertToU8 },
		{DT_INT16 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_INT32 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU32},

		{DT_UINT16 , "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_UINT32 , "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},
		{DT_INT16  , "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_INT32  , "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},

		{DT_FLOAT32, "%.2f", (DtConvertFn)Dt_convertToU32},
	};

	const char* fmt = Null;
	if (tagLen == 1)
	{
		fmt = "[%02X-%02d]:";
	}
	else if (tagLen == 2)
	{
		fmt = "[%04X-%02d]:";
	}

	//Sample: [120A-1]:2B(43)

	Printf(fmt, tag, len);
	fmt = Null;

	DtConvertFn convet;
	for (int i = 0; i < GET_ELEMENT_COUNT(disp); i++)
	{
		if (dt == disp[i].dt)
		{
			fmt = disp[i].fmt;
			convet = disp[i].Convert;
		}
	}

	if (fmt)
	{
		Printf(fmt, convet(val, dt));
	}
	else if (dt == DT_STRING)
	{
		Printf((char*)val);
	}
	else
	{
		DUMP_BYTE(val, len);
	}
}


void Tlvs_dump(const uint8* tlvs, uint8 tlvsLen, int tagLen)
{
	uint32 tag = 0;
	uint8 len = tlvs[tagLen];
	for (int i = 0; i < tlvsLen; i += tagLen + 1 + len)
	{
		memcpy(&tag, tlvs, tagLen);
		len = tlvs[tagLen];

		Tlv_dump(tag, tagLen, len , &tlvs[tagLen + 1], DT_BYTES);

		tlvs += tagLen + 1 + len;
	}
}
