#ifndef __ONE_LINE_H_
#define __ONE_LINE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define HWTIMER_DEV_NAME "timer3" /* 定 时 器 名称 */

#define NVC_LOOP 0xFF
#define NVC_PLAY(x) Nvc_Play(x, 1);

#define VOL_DEFAULT 5

typedef enum _NVC_AUDIO
{
	 NVC_BAT_PLUG_IN = 0	//3秒音乐+电池已插入。
	,NVC_BAT_PLUG_OUT		//3秒音乐+电池已拔出。
	,NVC_BAT_SOC_100		//3秒音乐+电量剩余100%。
	,NVC_BAT_SOC_90
	,NVC_BAT_SOC_80
	,NVC_BAT_SOC_70
	,NVC_BAT_SOC_60
	,NVC_BAT_SOC_50
	,NVC_BAT_SOC_40
	,NVC_BAT_SOC_30
	,NVC_BAT_SOC_20
	,NVC_BAT_SOC_10			//3秒音乐+电量剩余10%。
	,NVC_LOW_POWER			//3秒音乐+电池电量低，请尽快换电。
	,NVC_SPEED_15			//3秒音乐+请注意，当前时速已超过15公里。
	,NVC_SPEED_25			//3秒音乐+请注意，当前时速已超过25公里。
	,NVC_INFO				//滴滴提示音。
	,NVC_ILLEG_MOVING		//警告，警告，车辆非法移动。
	,NVC_BAT_UNSAFE			//警告，警告，电池有被盗风险。
	,NVC_WARNING			//叭叭告警声，1秒。
}NVC_AUDIO;

typedef struct _NvcItem
{
	uint8 cmd;
	uint8 repeatCounter;
	uint8 maxRepeat;
}NvcItem;

//void Nvc_Run(void);
void Nvc_Play(uint8 audioInd, uint8 maxRepeat);
//Bool Nvc_Sleep(void);
//void Nvc_Wakeup(void);
void Nvc_PlayEx(uint8 audioInd, uint8 maxRepeat, uint8 vol);
void Nvc_SetVol(uint8 vol);
void Nvc_Reset(void);
void SIF_ISR_ENABLE(void);
void SIF_ISR_DISABLE(void);
void Nvc_Init();
#ifdef __cplusplus
}
#endif

#endif



