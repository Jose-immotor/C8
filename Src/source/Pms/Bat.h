
#ifndef __BAT__H_
#define __BAT__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

	//电池结构体
	typedef struct _Bat
	{
		uint8_t  port;			//仓位号，从0开始
		uint8_t  isPresent;		//电池是否存在
		uint32_t soc;			//电池电量

		uint32_t actChgMA;		//实际的充电电流(从电池读回来的实际充电电流)
		uint32_t allocChgMA;	//当前分配充电电流
		uint32_t newPwrMA;		//计算出来新的充电电流
	}Bat;

#ifdef __cplusplus
}
#endif

#endif


