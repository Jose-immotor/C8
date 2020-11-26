
#ifndef __THRESHOLD__H_
#define __THRESHOLD__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define THRESHOLD_INNVALID (0xFFFFFFFF>>1)

	/*
							MORE_THEN_THRESHOLD AREA（大于阈值区）
	THRESHOLD_HIGH：--------------------------------------------
							THRESHOLD AREA（阈值区内）
	THRESHOLD_LOW： --------------------------------------------
							LASS_THEN_THRESHOLD AREA（小于阈值区）
	*/

	//值变化事件定义
	typedef enum
	{
		NOT_OVER_THRESHOLD = BIT_0,			//没有越过任何阈值
		DOWN_OVER_THRESHOLD_LOW = BIT_1,	//向下越过阈值下线
		DOWN_OVER_THRESHOLD_HIGH = BIT_2,	//向下越过阈值上线
		UP_OVER_THRESHOLD_LOW = BIT_3,		//向上越过阈值下线
		UP_OVER_THRESHOLD_HIGH = BIT_4,		//向上越过阈值上线
	}THRESHOLD_EVENT;

	//阈值区定义，采样值位域那个区域
	typedef enum
	{
		UNKNOWN_AREA,				//未知区域
		MORE_THEN_THRESHOLD_AREA,	//大于阈值区
		IN_THRESHOLD_AREA,			//阈值区内
		LASS_THEN_THRESHOLD_AREA,	//小于阈值区
	}THRESHOLD_AREA;

	THRESHOLD_EVENT Threshold_GetEvent(int oldVal, int newVal, int thresholdHigh, int thresholdLow, THRESHOLD_AREA* pArea);

	struct _ThresholdArea;
	typedef void (*ThresholdChangedFn)(void* pObj, struct _ThresholdArea* pThreshold, THRESHOLD_EVENT event);
	typedef struct _ThresholdArea
	{
		//OnChanged事件阈值, 和convertVal发生改变时，都要和该值作比较，判定是否触发OnChanged事件
		// 当值变大且越过lowThreshold ，触发事件UP_OVER_THRESHOLD_LOW。 
		// 当值变大且越过thresholdHigh，触发事件UP_OVER_THRESHOLD_HIGH。 
		// 当值变小且越过lowThreshold ，触发事件DOWN_OVER_THRESHOLD_LOW。 
		// 当值变小且越过thresholdHigh，触发事件DOWN_OVER_THRESHOLD_HIGH。 
		int thresholdHigh;	//阈值上线，无效值：THRESHOLD_INNVALID
		int thresholdLow;	//阈值下线, 无效值：THRESHOLD_INNVALID

		ThresholdChangedFn	OnChanged;	//值改变事件

		THRESHOLD_AREA thresholdArea;	//采样值convertVal位域那个阈值区。
		int oldVal;
		int newVal;
	}ThresholdArea;


#ifdef __cplusplus
}
#endif

#endif

