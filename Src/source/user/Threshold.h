
#ifndef __THRESHOLD__H_
#define __THRESHOLD__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define THRESHOLD_INNVALID (0xFFFFFFFF>>1)

	/*
							MORE_THEN_THRESHOLD AREA��������ֵ����
	THRESHOLD_HIGH��--------------------------------------------
							THRESHOLD AREA����ֵ���ڣ�
	THRESHOLD_LOW�� --------------------------------------------
							LASS_THEN_THRESHOLD AREA��С����ֵ����
	*/

	//ֵ�仯�¼�����
	typedef enum
	{
		NOT_OVER_THRESHOLD = BIT_0,			//û��Խ���κ���ֵ
		DOWN_OVER_THRESHOLD_LOW = BIT_1,	//����Խ����ֵ����
		DOWN_OVER_THRESHOLD_HIGH = BIT_2,	//����Խ����ֵ����
		UP_OVER_THRESHOLD_LOW = BIT_3,		//����Խ����ֵ����
		UP_OVER_THRESHOLD_HIGH = BIT_4,		//����Խ����ֵ����
	}THRESHOLD_EVENT;

	//��ֵ�����壬����ֵλ���Ǹ�����
	typedef enum
	{
		UNKNOWN_AREA,				//δ֪����
		MORE_THEN_THRESHOLD_AREA,	//������ֵ��
		IN_THRESHOLD_AREA,			//��ֵ����
		LASS_THEN_THRESHOLD_AREA,	//С����ֵ��
	}THRESHOLD_AREA;

	THRESHOLD_EVENT Threshold_GetEvent(int oldVal, int newVal, int thresholdHigh, int thresholdLow, THRESHOLD_AREA* pArea);

	struct _ThresholdArea;
	typedef void (*ThresholdChangedFn)(void* pObj, struct _ThresholdArea* pThreshold, THRESHOLD_EVENT event);
	typedef struct _ThresholdArea
	{
		//OnChanged�¼���ֵ, ��convertVal�����ı�ʱ����Ҫ�͸�ֵ���Ƚϣ��ж��Ƿ񴥷�OnChanged�¼�
		// ��ֵ�����Խ��lowThreshold �������¼�UP_OVER_THRESHOLD_LOW�� 
		// ��ֵ�����Խ��thresholdHigh�������¼�UP_OVER_THRESHOLD_HIGH�� 
		// ��ֵ��С��Խ��lowThreshold �������¼�DOWN_OVER_THRESHOLD_LOW�� 
		// ��ֵ��С��Խ��thresholdHigh�������¼�DOWN_OVER_THRESHOLD_HIGH�� 
		int thresholdHigh;	//��ֵ���ߣ���Чֵ��THRESHOLD_INNVALID
		int thresholdLow;	//��ֵ����, ��Чֵ��THRESHOLD_INNVALID

		ThresholdChangedFn	OnChanged;	//ֵ�ı��¼�

		THRESHOLD_AREA thresholdArea;	//����ֵconvertValλ���Ǹ���ֵ����
		int oldVal;
		int newVal;
	}ThresholdArea;


#ifdef __cplusplus
}
#endif

#endif

