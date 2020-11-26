#ifndef __ADC_DRIVER_H_
#define __ADC_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "Threshold.h"


	typedef enum
	{
		  ADC_18650_CHARGE_I	//PC5, ADC01_IN15 , 18650������������� ADC
		, ADC_18650_VOLTAGE 	//PB0, ADC01_IN8 , 18650��ص�ѹ��� ADC
		, ADC_18650_TEMP	 	//PB1, ADC01_IN9 , 18650����¶� ADC

		, ADC_TEMP_LCD			//PC0, ADC012_IN10, ���NTC���¶Ȳ�������ADC��, �ɼ�LCD
		, ADC_TEMP_CHGR			//PC1, ADC012_IN11, ���NTC���¶Ȳ�������ADC��
		//, ADC_TEMP_3			//PC2, ADC012_IN12, ���NTC���¶Ȳ�������ADC��
		//, ADC_TEMP_4			//PC3, ADC012_IN13, ���NTC���¶Ȳ�������ADC��

		, ADC_TEMP_MCU			//ADC0_CH16, �ڲ��¶ȴ�����
		, ADC_VERF_MCU			//ADC0_CH17, �ڲ��ο���ѹ VREFINT
	}ADC_ID;

	struct _Adc;
//	typedef void (*AdcChangedFn)(struct _Adc* pAdc, THRESHOLD_EVENT event, int oldVal, int newVal);
	typedef void (*AdcOnChangedFn)(struct _Adc* pAdc, int oldVal, int newVal);
	typedef struct _Adc
	{
		//���������������в������޸�////////////////////////////////////
		ADC_ID adcId;	//�ɼ���
		const char* name;	//����
		uint32 ioPort;		//Gpio Port
		uint16 ioPin;		//Gpio Pin
		uint32_t adcPort;	//Adc port��  ADCx,x=0,1,2
		uint8 adcChnl;		//Adcͨ��	, ADC_CHANNEL_x(x=0..17)(x=16 and x=17 are only for ADC0)
		uint16* pDmaRawValue;	//ָ��DMA�������Rawֵ��ָ��
		float refVoltage;	//�ο���ѹ

		//ָ�������ֵ������ָ�룬���Գ�ʼ�����ã���������һ
		uint8* odValue8;	//���OD������������Uint8ʱ ��ʹ�ô�ָ��
		uint16* odValue16;	//���OD������������Uint16ʱ��ʹ�ô�ָ��
		
		//���������������п����޸�////////////////////////////////////
		Bool isPresent;	//�Ƿ���λ
		int newValue;	//��ת��ֵ
		int rawValue;	//����ԭʼֵ
		int oldValue;	//��ת��ֵ

		void* pObj;
		int thresholdSize;	//��ֵ����Ԫ�ظ���
		ThresholdArea* pThresholdArray;	//��ֵ���飻
		AdcOnChangedFn OnChanged;
	}Adc;

	extern Adc* g_pAdc_18650Votage;
	//typedef struct
	//{
	//	AdcCfg adc0Cfg;
	//	AdcCfg adc1Cfg;

	//	uint32 port;
	//}Adc;

	void Adc_init();
	void Adc_Stop();
	void Adc_Start();
	Adc* Adc_Get(ADC_ID adcId);
	void Adc_AllStateChanged();
	void Adc_EventRegister(ADC_ID field, ThresholdArea* pThresholdArray, int thresholdSize, void* pObj);

#ifdef __cplusplus
}
#endif

#endif

