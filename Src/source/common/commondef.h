#ifndef __COMMON_DEF_H_
#define __COMMON_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "common.h"	
	
#define USERID_LEN  16
#define SID_LEN		12
#define CCID_LEN  	20
#define MD5_KEY_LEN 16

#define ENMU_LOCK   		0x01
#define ENMU_UNLOCK 		0x00

enum
{
	RTC_WAKEUP_TIMEID= 1
};

typedef enum _WakeupType
{
	 WAKEUP_POWER = 0
	,WAKEUP_GYRO
	,WAKEUP_PMS
	,WAKEUP_SIM
	,WAKEUP_BLE
	,WAKEUP_RTC
	,WAKEUP_LOCK
#ifdef CFG_CABIN_LOCK	
	,WAKEUP_CABIN_LOCK
#endif
	,WAKEUP_TAKE_APART
	,WAKEUP_MAX
}WakeupType;


#ifdef __cplusplus
}
#endif

#endif



