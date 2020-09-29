
#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "MsgDef.h"
#include "gd32e10x.h"

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
	CTRL_MCU_LED=0x01,		//PE11, MCU���е�
	IO_NFC_NPD_A,
	IO_NFC_IRQ_A,
	IO_GRYO_IRQ,
	IO_18650BOOST_EN,
	IO_NFC_PWR_OFF,
	IO_GPRS_INSERT,
	IO_18650_CHG_EN,
	IO_BAT_INSERT,
	IO_NVC_DATA,
	IO_NVC_BUSY,
	IO_NVC_PWR,
	IO_PWR3V3_EN,
	IO_PWR485_EN,
	IO_DIR485_CTRL,
	IO_TAKE_APART,
	IO_LOCK_EN,
	IO_OVER_TEMP,
	IO_18650_PWR_OFF,
	IO_CABIN_12V_ON,
	IO_CABIN_FB,
	IO_AT8837_IN1,
	IO_AT8837_IN2,
	IO_AT8837_nSLEEP
	
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

//void IO_Init();
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
int IO_Init(void);
#ifdef __cplusplus
}
#endif

#endif

