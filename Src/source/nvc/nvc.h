#ifndef __ONE_LINE_H_
#define __ONE_LINE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define HWTIMER_DEV_NAME "timer3" /* �� ʱ �� ���� */

#define NVC_LOOP 0xFF
#define NVC_PLAY(x) Nvc_Play(x, 1);

#define VOL_DEFAULT 5

typedef enum _NVC_AUDIO
{
	 NVC_BAT_PLUG_IN = 0	//3������+����Ѳ��롣
	,NVC_BAT_PLUG_OUT		//3������+����Ѱγ���
	,NVC_BAT_SOC_100		//3������+����ʣ��100%��
	,NVC_BAT_SOC_90
	,NVC_BAT_SOC_80
	,NVC_BAT_SOC_70
	,NVC_BAT_SOC_60
	,NVC_BAT_SOC_50
	,NVC_BAT_SOC_40
	,NVC_BAT_SOC_30
	,NVC_BAT_SOC_20
	,NVC_BAT_SOC_10			//3������+����ʣ��10%��
	,NVC_LOW_POWER			//3������+��ص����ͣ��뾡�컻�硣
	,NVC_SPEED_15			//3������+��ע�⣬��ǰʱ���ѳ���15���
	,NVC_SPEED_25			//3������+��ע�⣬��ǰʱ���ѳ���25���
	,NVC_INFO				//�ε���ʾ����
	,NVC_ILLEG_MOVING		//���棬���棬�����Ƿ��ƶ���
	,NVC_BAT_UNSAFE			//���棬���棬����б������ա�
	,NVC_WARNING			//�Ȱȸ澯����1�롣
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
#ifdef __cplusplus
}
#endif

#endif



