#ifndef __DRIVER_IO_H_
#define __DRIVER_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "MsgDef.h"
	
#define IS_OUT_MODE(mode) ((mode & 0xF0) == 0x10)
#define IS_IN_MODE(mode) ((mode & 0xF0) != 0x10)

#define FUN_IO_POWER_OFF() IO_Set(CTRL_FUN_POWER, 0)
#define FUN_IO_POWER_ON()  IO_Set(CTRL_FUN_POWER, 1)

#define LIGHT_POWER_OFF() IO_Set(CTRL_LIGHT, 0)
#define LIGHT_POWER_ON()  IO_Set(CTRL_LIGHT, 1)

//LCD ��/�ص�
#define LCD_POWER_OFF() IO_Set(CTRL_LCD_POWER, 0)
#define LCD_POWER_ON()  IO_Set(CTRL_LCD_POWER, 1)
//LCD��λ��
#define LCD_RESET_ON()  IO_Set(CTRL_LCD_RESET, 1)
#define LCD_RESET_OFF()  IO_Set(CTRL_LCD_RESET, 0)

#define PUMP_POWER_OFF() CO_OD_RAM.pump.powerOn = 0	//IO_Set(CTRL_PUMP, 0)
#define PUMP_POWER_ON()  CO_OD_RAM.pump.powerOn = 1 //IO_Set(CTRL_PUMP, 1)

//�̵����ϵ�Ӳ⣬n = DET_12V_1��DET_12V_2��DET_12V_3��DET_12V_4
#define RM_POWER_IS_ON(n) (IO_Read(n) == 0)

//�̵���ģ���ϵ磬n = CTRL_AC3_PMS��CTRL_AC4��CTRL_AC5
#define RM_POWER_ON(n)	IO_Set(n, 1)
#define RM_POWER_OFF(n)	IO_Set(n, 0)

#define IO_LOW  0
#define IO_HIGH 1

#define AC_ON  1
#define AC_OFF 0

#define PUMP_CTRL_ON  1	//ˮ�ÿ��ƿ���-����1-�ϵ硣0-�ص硣
#define PUMP_CTRL_OFF 0	//ˮ�ÿ��ƿ���-�أ�1-�ϵ硣0-�ص硣

#define FUN_OK	  1
#define FUN_FAULT 0

#define RELAY_12V_ON  0
#define RELAY_12V_OFF 1

#define BU_ON()  IO_Set(CTRL_12VBP_ON, IO_HIGH)
#define BU_OFF() IO_Set(CTRL_12VBP_ON, IO_LOW)

#define SET_18650BP_ON()  IO_Set(CTRL_18650BP_ON, IO_HIGH)
#define SET_18650BP_OFF() IO_Set(CTRL_18650BP_ON, IO_LOW)

#define BP_START_SOC_THRESHOLD 10	//�����ع���������ֵ��������ڸ�ֵ������������

	//�̵��������붨��
	typedef enum
	{
		RELAY_ERR_OK = 0,
		RELAY_ERR_ON,		//�̵�������, Ӧ��Ϊ�򿪣�ʵ�ʹر�
		RELAY_ERR_OFF,		//�̵�������Ӧ��Ϊ�رգ�ʵ�ʴ�
	}RELAY_ERR;

	//�ź����ƶ��壬��ö��ֵ���ܱ�д��LOG�ļ��У��޸���ö��ֵ���ܻᵼ��log�ļ����¼�CCU_ET_IO_DET_FAULT���ݴ���
	typedef enum
	{
		DET_BUCK_OFF = 0x01,	//PA0, ADC012_IN0 , 3V6 BUCK�����⣬ �͵�ƽ����
		DET_PUMP_OFF,		//PA15,ˮ��������⣬�͵�ƽ��ʾ����

		DET_AC_ON,			//PB3,�е�ϵ�ָʾ���ߵ�ƽ��ʾ�������͵�ƽ�ϵ�

		DET_SWITCH_BIT3 = 0x10,	//PD4,���뿪��, BIT3
		DET_SWITCH_BIT2,	//PD5,���뿪��, BIT2
		DET_SWITCH_BIT1,	//PD6,���뿪��, BIT1
		DET_SWITCH_BIT0,	//PD7,���뿪��, BIT0

		DET_FUN1_FAULT = 0x20,	//PD14,����1���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����
		DET_FUN2_FAULT,	//PD13,����2���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����
		DET_FUN3_FAULT,	//PD12,����3���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����
		DET_FUN4_FAULT,	//PD11,����4���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����
		DET_FUN5_FAULT,	//PD10,����1���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����
		DET_FUN6_FAULT,	//PB7, ����6���ϼ�⣬�͵�ƽ��ʾ���ϣ��ߵ�ƽ����

		DET_BLE_LOCK = 0x30,//PE4, ���������������ź�, ��ʱ����
		DET_WATER_IN,		//PE5, �����ˮ��⣬�ߵ�ƽ��ʾ��ˮ
		DET_BUCKET,			//PE6, ˮͰˮλ��⣬�͵�ƽ��ʾˮ������

		DET_12V_1 = 0x40,	//PE7, 12V_#1_DET, 1��12V�̵����Ƿ��е磬�͵�ƽ�е�, LCD,�������������豸��Դ�����ɿ�
		DET_12V_2,			//PE8, 12V_#2_DET, 2��12V�̵����Ƿ��е磬�͵�ƽ�е�, ��Ƶ���ϵͳ��������Դ�����ɿ�
		DET_12V_3,			//PE9, 12V_#3_DET, 3��12V�̵����Ƿ��е磬�͵�ƽ�е磬���ȣ���������磬�ɿ�
		DET_12V_4,			//PE10,12V_#4_DET, 4��12V�̵����Ƿ��е磬�͵�ƽ�е�, PMS���磬�ɿ�

		DET_12V_BP,			//PE12, �����Դש 12V�����⣬�͵�ƽ�е�
		DET_12V_18650BP,	//PE14, 18650��ѹ12V�����⣬�͵�ƽ�е�
		DET_BU_PRESENT,		//PD9, �������λ��⣬�ߵ�ƽ��ʾ��λ

		//Out put
		CTRL_FUN_POWER = 0x50,	//PA11, ���ȹ�����ƣ��ߵ�ƽ��Ч��Ӳ����·Ĭ�Ϲر�
		CTRL_LIGHT,				//PA12, �����ƿ��ƣ��ߵ�ƽ��Ч��Ӳ����·Ĭ�Ͽ���

		CTRL_485_METER_ENABLE,	//PA1, ���485ʹ�ܿ���
		CTRL_485_LOCK_ENABLE,	//PE15, 4����������85ʹ�ܿ���
		CTRL_485_COOLING_ENABLE,//PA8, �յ�485ʹ�ܿ���

		CTRL_MCU_LED,		//PE11, MCU���е�
		CTRL_CAN_RED,		//PD1, CAN OPEN״̨�ƣ���ɫ
		CTRL_CAN_GREEN,		//PD0, CAN OPEN״̨�ƣ���ɫ

		CTRL_LCD_POWER,		//PC11, LCD������ƣ��ߵ�ƽ��Ч��Ӳ����·Ĭ�Ͽ���
		CTRL_LCD_RESET,		//PE2, LCD ��λ���͵�ƽ��λ����n�룬������

		CTRL_PUMP = 0x60,	//PC10, ˮ�ù�����ƣ��ߵ�ƽ��Ч��Ӳ����·Ĭ�Ϲر�
		CTRL_AC3_PMS,		//PE1, �̵���3�ϵ磬�ߵ�ƽ��Ч��Ӳ����·Ĭ�Ϲرգ�PMS����
//		CTRL_AC4,			//PE0, ����
//		CTRL_AC5,			//PB4, ����

		CTRL_18650BP_ON=0x70,	//PB15, BU_CTR2, ����18650BP���ߵ�ƽʹ�ܷŵ�
		CTRL_12VBP_ON ,		//PD8, ����back_up��12V����ģ����ƣ��ߵ�ƽʹ�ܷŵ�

		CTRL_BU_CHARGE,		//PB14, ����back_up��磬�ߵ�ƽʹ�ܳ��
		CTRL_HOT_FUN,		//PC13, ���Ƶ�Դ�������ȷ��ȣ��ߵ�ƽ��Ч
		CTRL_18650_CHG_EN,	//PE13, 18650��繩����ƣ��ߵ�ƽ��Ч��Ӳ����·Ĭ�Ϲر�
	}IO_ID;

#define RM_POWER_FUN DET_12V_3
#define RM_POWER_PMS DET_12V_4

	typedef void (*DrvIoStateChanged)(IO_ID ioID, Bool isHigh);
	typedef struct
	{
		IO_ID ioID;
		const char* desc;	//IO�����ƣ�������Բ鿴

		/* GPIOx(x=A,B,C,D,E,F,G) definitions */
		uint32 periph;	

		/* GPIO pin definitions 
		GPIO_PIN_0 
		GPIO_PIN_1 
		GPIO_PIN_2 
		GPIO_PIN_3 
		GPIO_PIN_4 
		GPIO_PIN_5 
		GPIO_PIN_6 
		GPIO_PIN_7 
		GPIO_PIN_8 
		GPIO_PIN_9 
		GPIO_PIN_10
		GPIO_PIN_11
		GPIO_PIN_12
		GPIO_PIN_13
		GPIO_PIN_14
		GPIO_PIN_15
		*/
		uint32 pin;

		/* GPIO mode definitions 
		GPIO_MODE_AIN              
		GPIO_MODE_IN_FLOATING      
		GPIO_MODE_IPD              
		GPIO_MODE_IPU              
		GPIO_MODE_OUT_OD           
		GPIO_MODE_OUT_PP           
		GPIO_MODE_AF_OD            
		GPIO_MODE_AF_PP        
		*/    
		uint8 mode;

		exti_line_enum line;	//EXTI�ص㴥���ߣ�0 ��ʾ��Чֵ��

		/*portSource: gpio event output port
		GPIO_PORT_SOURCE_GPIOA: output port source A
		GPIO_PORT_SOURCE_GPIOB: output port source B
		GPIO_PORT_SOURCE_GPIOC: output port source C
		GPIO_PORT_SOURCE_GPIOD: output port source D
		GPIO_PORT_SOURCE_GPIOE: output port source E 
		GPIO_PORT_SOURCE_GPIOF: output port source F
		GPIO_PORT_SOURCE_GPIOG: output port source G
		*/
		uint8 portSource;

		/*pinSource: GPIO_PIN_SOURCE_x(x=0..15)*/
		uint8 pinSource;

		/* trig_type: interrupt trigger type, refer to exti_trig_type_enum
			only one parameter can be selected which is shown as below :
			EXTI_TRIG_RISING : rising edge trigger
			EXTI_TRIG_FALLING : falling trigger
			EXTI_TRIG_BOTH : rising and falling trigger
		*/
		exti_trig_type_enum trig_type;	//��������

		MSG_ID stateChangedMsg;	//IO״̬�ı�ʱ��������Ϣֵ
		uint8* pOdValue;

		//IO��״ֵ̬��0����1
		uint8 value;		//����ֵ

		//Output Mode�����ڼ����ϼ�ʱ��
		//Input Mode�����ڷ�����ʱ��
		uint32 ticks;	//0:��ʾ��ֹ������⣬> 0:��������������ʼʱ��
		uint32 ticksDelay;	//���������ʱʱ�䣬Ms

		//�Ƿ���ϣ�����ط���IO��״̬ȷ��
		Bool isFault;

		Bool isEnable;
	}DrvIo;
	extern uint8 g_ToggleValue;
	extern DrvIo* g_pBucket;			//ˮͰˮλ����״̬
	extern DrvIo* g_pWaterIn;			//����ˮλ����״̬
	extern DrvIo* g_pPumpCtrl;			//ˮ�ÿ���
	extern DrvIo* g_pLightCtrl;			//�����ƿ���
	extern DrvIo* g_pLcdResetCtrl;		//LCD Reset

	void IO_Init();
	void IO_Dump(DrvIo* p, const char* head);
	void IOs_Dump();

	DrvIo* IO_Get(IO_ID io);

	uint8 PortPin_Read(const DrvIo* pDrvIo);
	//portPinName = "PA1","PB2"..."PE15"
	uint8 IODesc_Read(const char* portPinName);
	uint8 IO_Read(IO_ID pin);

	void PortPin_Set(uint32 port, uint32 pin, uint8 value);
	//portPinName = "PA1","PB2"...
	//value = 0;1
	void IODesc_Set(const char* portPinName, uint8 value);
	void IO_Set(IO_ID pin, uint8 value);
	void IO_AllStateChanged();
	void IO_TriggerStateChanged(IO_ID pin);
	char* PortPinToPxx(uint32 port, uint32 pin);

	void IO_SetRunLed(uint16 onMs, uint16 offMs);
	char* PortPinToPxx(uint32 port, uint32 pin);

#ifdef __cplusplus
}
#endif

#endif

