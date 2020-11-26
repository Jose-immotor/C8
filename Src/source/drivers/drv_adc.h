#ifndef __ADC_DRIVER_H_
#define __ADC_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "Threshold.h"


	typedef enum
	{
		  ADC_18650_CHARGE_I	//PC5, ADC01_IN15 , 18650充电输入电流检测 ADC
		, ADC_18650_VOLTAGE 	//PB0, ADC01_IN8 , 18650电池电压检测 ADC
		, ADC_18650_TEMP	 	//PB1, ADC01_IN9 , 18650电池温度 ADC

		, ADC_TEMP_LCD			//PC0, ADC012_IN10, 外接NTC，温度采样，接ADC脚, 采集LCD
		, ADC_TEMP_CHGR			//PC1, ADC012_IN11, 外接NTC，温度采样，接ADC脚
		//, ADC_TEMP_3			//PC2, ADC012_IN12, 外接NTC，温度采样，接ADC脚
		//, ADC_TEMP_4			//PC3, ADC012_IN13, 外接NTC，温度采样，接ADC脚

		, ADC_TEMP_MCU			//ADC0_CH16, 内部温度传感器
		, ADC_VERF_MCU			//ADC0_CH17, 内部参考电压 VREFINT
	}ADC_ID;

	struct _Adc;
//	typedef void (*AdcChangedFn)(struct _Adc* pAdc, THRESHOLD_EVENT event, int oldVal, int newVal);
	typedef void (*AdcOnChangedFn)(struct _Adc* pAdc, int oldVal, int newVal);
	typedef struct _Adc
	{
		//以下内容在运行中不可以修改////////////////////////////////////
		ADC_ID adcId;	//采集域
		const char* name;	//名称
		uint32 ioPort;		//Gpio Port
		uint16 ioPin;		//Gpio Pin
		uint32_t adcPort;	//Adc port，  ADCx,x=0,1,2
		uint8 adcChnl;		//Adc通道	, ADC_CHANNEL_x(x=0..17)(x=16 and x=17 are only for ADC0)
		uint16* pDmaRawValue;	//指向DMA采样后的Raw值的指针
		float refVoltage;	//参考电压

		//指向数据字典的数据指针，来自初始化配置，二者用其一
		uint8* odValue8;	//如果OD的数据类型是Uint8时 ，使用此指针
		uint16* odValue16;	//如果OD的数据类型是Uint16时，使用此指针
		
		//以下内容在运行中可以修改////////////////////////////////////
		Bool isPresent;	//是否在位
		int newValue;	//新转换值
		int rawValue;	//采样原始值
		int oldValue;	//旧转换值

		void* pObj;
		int thresholdSize;	//阈值数组元素个数
		ThresholdArea* pThresholdArray;	//阈值数组；
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

