/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TLV__H_
#define __TLV__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

	void Tlvs_dump(const uint8* tlvs, int tlvsLen, int tagLen);

#ifdef __cplusplus
}
#endif

#endif
