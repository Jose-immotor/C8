/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"

void Tlvs_dump(const uint8* tlvs, int tlvsLen, int tagLen)
{
	uint32 tag = 0;
	uint8 len = tlvs[tagLen];
	for (int i = 0; i < tlvsLen; i += tagLen + 1 + len)
	{
		memcpy(&tag, tlvs, tagLen);
		len = tlvs[tagLen];

		Printf("[0x%X-%d]: ", tag, len);
		DUMP_BYTE(&tlvs[tagLen+1], len);

		tlvs += tagLen + 1 + len;
	}
}
