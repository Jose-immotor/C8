#ifndef __BMS_FAULT_H_
#define __BMS_FAULT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "Battery.h"

#define DEVFT1_ALARM_MASK (uint16)(BIT_0 | BIT_4 | BIT_5)
#define DEVFT2_ALARM_MASK (uint16)(BIT_0 | BIT_1 | BIT_3 | BIT_4 | BIT_5)
#define OPFT1_ALARM_MASK (uint16)(BIT_1 | BIT_2 | BIT_4 | BIT_5 | BIT_7| BIT_9| BIT_10 | BIT_11 | BIT_12 | BIT_14 | BIT_15)
#define OPFT2_ALARM_MASK (uint16)(BIT_0 | BIT_3 | BIT_4)

#pragma pack(1)
typedef struct _BmsFault
{
	uint8 serialNum[SERIAL_NUM_SIZE];
	uint16 devFt1;
	uint16 devFt2;
	uint16 opFt1;
	uint16 opFt2;
	uint16 opWart1;
	uint16 opWart2;
}BmsFault;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif


