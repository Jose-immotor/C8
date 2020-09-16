#ifndef __ADC_USER_H_
#define __ADC_USER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "drv_adc.h"

//#define UP_OVER_THRESHOLD_LOW

	typedef struct
	{
		uint32 id;
		uint32 event;	//高线事件
	}AdcUser;
	//THRESHOLD_EVENT AdcUser_GetTempEvent();
	extern AdcUser* g_pLcdTempSensor ;
	extern AdcUser* g_pChgrTempSensor;

	void AdcUser_Init();
#ifdef __cplusplus
}
#endif

#endif

