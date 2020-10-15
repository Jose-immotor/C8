
#include "Common.h"
#include "AdcUser.h"
//#include "HighTempAlarm.h"

//���ڼ���¶ȱ仯�ۺ��жϣ�����ÿ�����������¼�ֵ
static AdcUser g_AdcUser[] = 
{
	{ADC_18650_VOLTAGE , NOT_OVER_THRESHOLD},
//	{ADC_TEMP_CHGR, NOT_OVER_THRESHOLD},
};

AdcUser* g_pLcdTempSensor = &g_AdcUser[0];
AdcUser* g_pChgrTempSensor = &g_AdcUser[1];

DrvIo* g_p18650ChgIO = Null;

THRESHOLD_EVENT AdcUser_GetTempEvent(uint8 sensor)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_AdcUser); i++)
	{
		if (sensor == g_AdcUser[0].id)
			return g_AdcUser[0].event;
	}
	return NOT_OVER_THRESHOLD;
}

//������е��¶ȴ������¼�
//��һ���¶ȱ���¼���������һ������������
//�¶ȱ���¼����������е��¶ȴ������¶�ֵ��ͣ����ܴ�������¼���
//THRESHOLD_EVENT AdcUser_GetTempEvent()
//{
//	THRESHOLD_EVENT eventMsg = NOT_OVER_THRESHOLD;
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_AdcUser); i++)
//	{
//		eventMsg |= g_AdcUser[i].event;
//	}
//
//	//����Ƿ���Խ�������ֵ�ߣ�
//	if (eventMsg)
//	{
//		if (eventMsg & UP_OVER_THRESHOLD_HIGH)
//		{
//			eventMsg = UP_OVER_THRESHOLD_HIGH;
//		}
//		else if (eventMsg & DOWN_OVER_THRESHOLD_LOW)
//		{
//			eventMsg = DOWN_OVER_THRESHOLD_HIGH;
//		}
//	}
//	else
//	{
//		//����Ƿ���Խ�������ֵ�ߣ�
//		for (int i = 0; i < GET_ELEMENT_COUNT(g_AdcUser); i++)
//		{
//			eventMsg |= g_AdcUser[i].event;
//		}
//
//		if (eventMsg & UP_OVER_THRESHOLD_HIGH)
//		{
//			eventMsg = UP_OVER_THRESHOLD_LOW;
//		}
//		else if (eventMsg & DOWN_OVER_THRESHOLD_LOW)
//		{
//			eventMsg = DOWN_OVER_THRESHOLD_LOW;
//		}
//	}
//
//	return eventMsg;
//}

void AdcUser_OnTempChanged(Adc* pAdc, int oldVal, int newVal)
{
	uint8 adcId = pAdc->adcId;
//	if (adcId == ADC_TEMP_LCD)
//	{
//		HighTempAlarm_SetEvent(adcId, 0, (newVal >= g_pCfgInfo->lcdAreaTemp));
//	}
//	else if (adcId == ADC_TEMP_CHGR)
//	{
//		HighTempAlarm_SetEvent(adcId, 0, (newVal >= g_pCfgInfo->chgAreaTemp));
//	}
}

void AdcUser_18650EventChanged(AdcUser* pAdcUser, ThresholdArea* pThreshold, THRESHOLD_EVENT event)
{
	//�����¶ȴ������¼�ֵ

	if (event == NOT_OVER_THRESHOLD) return;

	//�����������¼�
	if (event == UP_OVER_THRESHOLD_LOW) return;		
	if (event == DOWN_OVER_THRESHOLD_HIGH) return;

	if (event == DOWN_OVER_THRESHOLD_LOW)
	{
		PortPin_Set(g_p18650ChgIO->periph, g_p18650ChgIO->pin, False);
	}
	else if (event == UP_OVER_THRESHOLD_HIGH)
	{
		PortPin_Set(g_p18650ChgIO->periph, g_p18650ChgIO->pin, True);
	}
//	if (pThreshold->thresholdHigh == g_pCfgInfo->temp_thresholdHigh)
//	{
//		if (event == DOWN_OVER_THRESHOLD_LOW)
//			event = DOWN_OVER_THRESHOLD_HIGH;
//	}
//	else
//	{
//		if (event == UP_OVER_THRESHOLD_HIGH)
//			event = UP_OVER_THRESHOLD_LOW;
//	}
	pAdcUser->event = event;

	//eventMsg = AdcUser_GetTempEvent();
	//PostMsgEx(MSG_TEMP_CHANGED, 0, eventMsg);
}

void AdcUser_Init()
{
	static ThresholdArea g_threshold[1];
	memset(g_threshold, 0, sizeof(g_threshold));

	//����ж༶��ֵ������ֵֵ������ǰ������ֵ�����ں�
	g_threshold[0].thresholdHigh = 4100;
	g_threshold[0].thresholdLow  = 3400;
	g_threshold[0].OnChanged     = (ThresholdChangedFn)AdcUser_18650EventChanged;

//	g_threshold[1].thresholdHigh = g_pCfgInfo->temp_thresholdLow;
//	g_threshold[1].thresholdLow  = g_pCfgInfo->temp_thresholdLow - g_pCfgInfo->temp_backlash;
//	g_threshold[1].OnChanged     = (ThresholdChangedFn)AdcUser_TempEventChanged;

	for (int i = 0; i < GET_ELEMENT_COUNT(g_AdcUser); i++)
	{
		Adc_EventRegister(g_AdcUser[i].id, g_threshold, GET_ELEMENT_COUNT(g_threshold), &g_AdcUser[i]);
		Adc* pAdc = Adc_Get(g_AdcUser[i].id);
		pAdc->OnChanged = (AdcOnChangedFn)AdcUser_OnTempChanged;
	}
	
	g_p18650ChgIO = IO_Get(IO_18650_CHG_EN);
}