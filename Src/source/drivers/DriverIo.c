
#include "Common.h"
#include "DriverIo.h"
#include "CO_OD.h"
#include "Led.h"

static Bool g_isIoStart = False;
uint8 g_ToggleValue = 0;

DrvIo* g_pPumpCtrl = Null;
DrvIo* g_pBucket;
DrvIo* g_pWaterIn;
DrvIo* g_pLightCtrl = Null;
DrvIo* g_pLcdResetCtrl = Null;

static DrvIo g_SwitchIOs[] =
{
	 {DET_SWITCH_BIT3, "SWITCH_BIT3", GPIOD, GPIO_PIN_4, GPIO_MODE_IN_FLOATING}
	,{DET_SWITCH_BIT2, "SWITCH_BIT2", GPIOD, GPIO_PIN_5, GPIO_MODE_IN_FLOATING}
	,{DET_SWITCH_BIT1, "SWITCH_BIT1", GPIOD, GPIO_PIN_6, GPIO_MODE_IN_FLOATING}
	,{DET_SWITCH_BIT0, "SWITCH_BIT0", GPIOD, GPIO_PIN_7, GPIO_MODE_IN_FLOATING}
};

//��������������ON����ʾ�ߵ�ƽ��Ч, ������OFF����ʾ�͵�ƽ��Ч
static DrvIo g_InIOs[] =
{
	 {DET_PUMP_OFF	 , "DET_PUMP_OFF", GPIOA, GPIO_PIN_15, GPIO_MODE_IN_FLOATING, EXTI_15, GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_15, EXTI_TRIG_BOTH , MSG_IO_PUMP, &OD_pumpPowerOn}
	,{DET_BUCK_OFF	 , "DET_BUCK_OFF", GPIOA, GPIO_PIN_0 , GPIO_MODE_IN_FLOATING, EXTI_0 , GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0 , EXTI_TRIG_BOTH , MSG_IO_BUCK}
	,{DET_AC_ON		 , "DET_AC_ON"  , GPIOB, GPIO_PIN_3 , GPIO_MODE_IN_FLOATING, EXTI_3 , GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_3 , EXTI_TRIG_BOTH , MSG_IO_AC  }

	,{DET_FUN1_FAULT, "DET_FUN1_OK", GPIOD, GPIO_PIN_14, GPIO_MODE_IN_FLOATING, EXTI_14, GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_14, EXTI_TRIG_BOTH, MSG_IO_FUN, &OD_fun1Fault}
	,{DET_FUN2_FAULT, "DET_FUN2_OK", GPIOD, GPIO_PIN_13, GPIO_MODE_IN_FLOATING, EXTI_13, GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_13, EXTI_TRIG_BOTH, MSG_IO_FUN, &OD_fun2Fault}
	,{DET_FUN3_FAULT, "DET_FUN3_OK", GPIOD, GPIO_PIN_12, GPIO_MODE_IN_FLOATING, EXTI_12, GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_12, EXTI_TRIG_BOTH, MSG_IO_FUN, &OD_fun3Fault}
	,{DET_FUN4_FAULT, "DET_FUN4_OK", GPIOD, GPIO_PIN_11, GPIO_MODE_IN_FLOATING, EXTI_11, GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_11, EXTI_TRIG_BOTH, MSG_IO_FUN, &OD_fun4Fault}
	,{DET_FUN5_FAULT, "DET_FUN5_OK", GPIOD, GPIO_PIN_10, GPIO_MODE_IN_FLOATING, (exti_line_enum)0, 0, 0, (exti_trig_type_enum)0, MSG_IO_FUN, &OD_fun5Fault}
	,{DET_FUN6_FAULT, "DET_FUN6_OK", GPIOB, GPIO_PIN_7, GPIO_MODE_IN_FLOATING , (exti_line_enum)0, 0, 0, (exti_trig_type_enum)0, MSG_IO_FUN, &OD_fun6Fault}

	,{DET_BU_PRESENT, "DET_BU_PRESENT", GPIOD, GPIO_PIN_9, GPIO_MODE_IN_FLOATING , (exti_line_enum)0, 0, 0, (exti_trig_type_enum)0, 0, &BU_isPresent}

	,{DET_BLE_LOCK	 ,"DET_BleLock", GPIOE, GPIO_PIN_4 , GPIO_MODE_IN_FLOATING, EXTI_4 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_4 , EXTI_TRIG_BOTH}
	,{DET_WATER_IN	 ,"DET_JiGuiWaterIn", GPIOE, GPIO_PIN_5 , GPIO_MODE_IN_FLOATING, EXTI_5 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_5 , EXTI_TRIG_BOTH, MSG_IO_WATER , &OD_shelfWaterIn}
	,{DET_BUCKET	 ,"DET_BucketFull" , GPIOE, GPIO_PIN_6 , GPIO_MODE_IN_FLOATING, EXTI_6 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_6 , EXTI_TRIG_BOTH, MSG_IO_BUCKET, &OD_waterLevel}
	,{DET_12V_BP	 ,"DET_12VBP_OFF"	, GPIOE, GPIO_PIN_12 , GPIO_MODE_IN_FLOATING, (exti_line_enum)0, 0, 0, (exti_trig_type_enum)0, MSG_IO_12VBP}
	,{DET_12V_18650BP,"DET_18650BP_OFF"	, GPIOE, GPIO_PIN_14 , GPIO_MODE_IN_FLOATING, (exti_line_enum)0, 0, 0, (exti_trig_type_enum)0, MSG_IO_18650BP}

	,{DET_12V_1		 ,"DET_12V1_OFF", GPIOE, GPIO_PIN_7 , GPIO_MODE_IN_FLOATING, EXTI_7 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_7 , EXTI_TRIG_BOTH, MSG_IO_12V, &OD_relay1PowerOff}
	,{DET_12V_2		 ,"DET_12V2_OFF", GPIOE, GPIO_PIN_8 , GPIO_MODE_IN_FLOATING, EXTI_8 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_8 , EXTI_TRIG_BOTH, MSG_IO_12V, &OD_relay2PowerOff}
	,{DET_12V_3		 ,"DET_12V3_OFF", GPIOE, GPIO_PIN_9 , GPIO_MODE_IN_FLOATING, EXTI_9 , GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_9 , EXTI_TRIG_BOTH, MSG_IO_12V, &OD_relay3PowerOff}
	,{DET_12V_4		 ,"DET_12V4_OFF", GPIOE, GPIO_PIN_10, GPIO_MODE_IN_FLOATING, EXTI_10, GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_10, EXTI_TRIG_BOTH, MSG_IO_12V, &OD_relay4PowerOff}
};					  

//��������������ON����ʾ�ߵ�ƽ��Ч, ������OFF����ʾ�͵�ƽ��Ч
static DrvIo g_OutputIOs[] =
{
	//�������
	 {CTRL_AC3_PMS  , "CTRL_AC3_PMS_ON"		, GPIOE, GPIO_PIN_1  ,GPIO_MODE_OUT_PP}
//	,{CTRL_AC4		, "CTRL_12V4_ON"		, GPIOE, GPIO_PIN_0  ,GPIO_MODE_OUT_PP, 0, 0, 0, 0, 0, 0, 0}
//	,{CTRL_AC5		, "CTRL_12V5_ON"		, GPIOB, GPIO_PIN_4  ,GPIO_MODE_OUT_PP, 0, 0, 0, 0, 0, 0, 0}
	,{CTRL_PUMP		, "CTRL_Pump_ON"		, GPIOC, GPIO_PIN_10 ,GPIO_MODE_OUT_PP, 0, 0, 0, 0, 0, &CO_OD_RAM.pump.powerOn}
	,{CTRL_LCD_POWER, "CTRL_LCD_POWER_ON"	, GPIOC, GPIO_PIN_11 ,GPIO_MODE_OUT_PP}
	,{CTRL_LCD_RESET, "CTRL_LCD_RESET"		, GPIOE, GPIO_PIN_2  ,GPIO_MODE_OUT_PP}
	,{CTRL_LIGHT	, "CTRL_Light_ON"		, GPIOA, GPIO_PIN_12 ,GPIO_MODE_OUT_PP, 0, 0, 0, 0, 0, &CO_OD_RAM.cabinet.lightPowerOn}
	,{CTRL_FUN_POWER, "CTRL_FunPower_ON"	, GPIOA, GPIO_PIN_11 ,GPIO_MODE_OUT_PP}

	,{CTRL_BU_CHARGE	, "CTRL_BU_CHARGE_ON", GPIOB, GPIO_PIN_14 ,GPIO_MODE_OUT_PP}
	,{CTRL_12VBP_ON		, "CTRL_12VBP_ON"	 , GPIOD, GPIO_PIN_8  ,GPIO_MODE_OUT_PP}
	,{CTRL_18650BP_ON	, "CTRL_18650BP_ON"	 , GPIOB, GPIO_PIN_15 ,GPIO_MODE_OUT_PP}
	,{CTRL_HOT_FUN		, "CTRL_HOT_FUN_ON"	 , GPIOC, GPIO_PIN_13 ,GPIO_MODE_OUT_PP}
	
	,{CTRL_MCU_LED	, "CTRL_MCU_LED"	, GPIOE, GPIO_PIN_11 ,GPIO_MODE_OUT_PP}
	,{CTRL_CAN_RED	, "CTRL_CAN_RED"	, GPIOD, GPIO_PIN_1  ,GPIO_MODE_OUT_PP}
	,{CTRL_CAN_GREEN, "CTRL_CAN_GREEN"	, GPIOD, GPIO_PIN_0  ,GPIO_MODE_OUT_PP}

	,{CTRL_485_METER_ENABLE	, "CTRL_485_METER"		, GPIOA, GPIO_PIN_1  ,GPIO_MODE_OUT_PP}
	,{CTRL_485_LOCK_ENABLE	, "CTRL_485_LOCK"		, GPIOE, GPIO_PIN_15 ,GPIO_MODE_OUT_PP}
	//,{CTRL_485_COOLING_ENABLE, "CTRL_485_COOLING"	, GPIOA, GPIO_PIN_8  ,GPIO_MODE_OUT_PP}	//��TIME0_CH0��PA8���ã����ڴ���ADC���
	,{CTRL_18650_CHG_EN		, "CTRL_18650_CHG_EN"	, GPIOE, GPIO_PIN_13 ,GPIO_MODE_OUT_PP}	
};

//=============================================
//����IO�ͷ���IO״̬ƥ�䶨��,
typedef struct
{
	IO_ID ctrlPin;		//����IO
	uint8 ctrlPinState;	//����IO״̬

	IO_ID detPin;		//����IO
	uint8 detPinState;	//����IO״̬

	MSG_ID msgId;		//�������ctrlPinState��detPinState��״̬��һ�£�������Ϣ

	DrvIo* pCtrlIo;		//ָ�����IO����
	DrvIo* pDetIo;		//ָ����IO����

	uint32 ticks;		//���ڼ��״̬IO��ʱ
	uint8 ctlPinCurrentVal;	//ctlPin��ǰֵ,�����ж��Ƿ����ı�
	uint8 detPinCurrentVal;	//detPin��ǰֵ,�����ж��Ƿ����ı�
	Bool  isFault;		//�Ƿ��д���
}IOPair;

static IOPair g_IOPairs[] =
{
	{DET_AC_ON		  , IO_HIGH, DET_12V_1	    , IO_LOW, MSG_IO_FAULT},
	{DET_AC_ON		  , IO_HIGH, DET_12V_2	    , IO_LOW, MSG_IO_FAULT},
	{CTRL_AC3_PMS	  , IO_HIGH, DET_12V_3	    , IO_LOW, MSG_IO_FAULT},
	{CTRL_AC3_PMS	  , IO_HIGH, DET_12V_4	    , IO_LOW, MSG_IO_FAULT},
	{CTRL_18650BP_ON  , IO_HIGH, DET_12V_18650BP, IO_LOW, MSG_IO_FAULT},
	{CTRL_12VBP_ON	  , IO_HIGH, DET_12V_BP	    , IO_LOW, MSG_IO_FAULT},
	{CTRL_PUMP		  , IO_HIGH, DET_PUMP_OFF   , IO_LOW, MSG_IO_FAULT},
};

void IO_IRQHandler(DrvIo* p);

const IOPair* IOPair_Get(IO_ID pin)
{
	const IOPair* p = g_IOPairs;
	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++, p++)
	{
		if (p->ctrlPin == pin) return p;
	}
	return Null;
}

void IO_Dump(DrvIo* p, const char* head)
{
	if (head == Null) head = "";
	Printf("%s%s-%s: %d\n", head, PortPinToPxx(p->periph, p->pin), p->desc, PortPin_Read(p));
}

void IOs_Dump()
{
	Bool isGoIn = False;
	Printf("IO Dump:\n")
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		IO_Dump(&g_InIOs[i], "\t");
		//Printf("\t%s-%s: \t%d\n", PortPinToPxx(g_InIOs[i].periph, g_InIOs[i].pin), g_InIOs[i].desc, g_InIOs[i].value);
	}
	Printf("\n");
	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		IO_Dump(&g_OutputIOs[i], "\t");
		//Printf("\t%s-%s: \t%d\n", PortPinToPxx(g_OutputIOs[i].periph, g_OutputIOs[i].pin), g_OutputIOs[i].desc, g_OutputIOs[i].value);
	}

	Printf("IOs Fault Dump:\n")
	const IOPair* p = g_IOPairs;
	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++, p++)
	{
		if (p->isFault)
		{
			isGoIn = True;
			IO_Dump(p->pCtrlIo, "\t");
			IO_Dump(p->pDetIo, "\t");
		}
	}
	if (!isGoIn)
	{
		Printf("No fault\n");
	}
}

void IOPair_CheckFault(IOPair* pPair)
{
	DrvIo* pCtrlIo = pPair->pCtrlIo;
	DrvIo* pDet = pPair->pDetIo;
	Bool isCheck = False;

	//�������ģʽ��IO�ţ���Ҫ��ʱ��⡣
	//��������ģʽ��IO�ţ�����Ҫ��ʱ��⣬�������������޸�ticks��valueֵ�������Ӱ������IO�ķ����ж��߼���
	if (IS_OUT_MODE(pCtrlIo->mode))
	{
		//������ֿ���IO���߼��IO��״̬�����仯������ʱ���2��IO��״̬�Ƿ�ƥ��
		//�����ƥ�䣬˵����������
		if (pPair->ctlPinCurrentVal != PortPin_Read(pCtrlIo)
			|| pPair->detPinCurrentVal != PortPin_Read(pDet))
		{
			pCtrlIo->ticks = GET_TICKS();
			pPair->ctlPinCurrentVal = PortPin_Read(pCtrlIo);
			pCtrlIo->value = PortPin_Read(pCtrlIo);
		}
		if (pPair->detPinCurrentVal != PortPin_Read(pDet))
		{
			pCtrlIo->ticks = GET_TICKS();
			pPair->detPinCurrentVal = PortPin_Read(pDet);
		}
		isCheck = (pCtrlIo->ticks && SwTimer_isTimerOutEx(pCtrlIo->ticks, 500));
		if(isCheck) pCtrlIo->ticks = 0;
	}
	else
	{
		//���CtrlIoֵ�仯����DetIoֵ�仯������ִ����������롣
		if (pPair->ctlPinCurrentVal != PortPin_Read(pCtrlIo)
			|| pPair->detPinCurrentVal != PortPin_Read(pDet))
		{
			isCheck = True;
			pPair->ctlPinCurrentVal = PortPin_Read(pCtrlIo);
			pPair->detPinCurrentVal = PortPin_Read(pDet);
		}
	}

	//��ʱ500ms���
	if (isCheck)
	{
		IO_IRQHandler(pDet);
		if (pPair->ctrlPinState == pPair->detPinState)
		{
			//�ж����úͷ����Ƿ�ƥ��
			pPair->isFault = (pDet->value != pCtrlIo->value);
		}
		else
		{
			//�ж����úͷ����Ƿ�ƥ��
			pPair->isFault = (pDet->value == pCtrlIo->value);
		}

		if (pPair->isFault)
		{
			LOG_TRACE1(CCU_ADDR, CCU_CATID_IO, pDet->ioID, CCU_ET_IO_DET_FAULT, pDet->value);
			PostMsgEx(pPair->msgId, pPair->ctrlPin, pPair->detPin);
			PFL(DL_MAIN, "%s is fault, \n", pDet->desc);
		}
	}
}
//===========================================================================
//perith������ƥ�䶨��
typedef struct
{
	uint32 periph;
	const char* Name;
}IOPortName;
const static IOPortName g_IOPortName[] =
{
	{GPIOA,"PA"},
	{GPIOB,"PB"},
	{GPIOC,"PC"},
	{GPIOD,"PD"},
	{GPIOE,"PE"},
	{GPIOF,"PF"},
	{GPIOG,"PG"},
};

//GPIOA->"PA"
const char* GPIOxToPx(uint32 port)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPortName); i++)
	{
		if (g_IOPortName[i].periph == port)
		{
			return g_IOPortName[i].Name;
		}
	}
	return "No";
}

//ch = 'A', 'B', 'C', 'D', 'E', 'F', 'G',
uint32 ABCDEFG_ToPort(char ch)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPortName); i++)
	{
		if (g_IOPortName[i].Name[1] == ch)
		{
			return g_IOPortName[i].periph;
		}
	}
	return 0;
}

//����port��pin������ת��Ϊ�ַ��������� PortNameToStr(GPIOA, GPIO_PIN_3) -> "PA3"
//����ֵ��"PA0";"PA1"..."PG15"
char* PortPinToPxx(uint32 port, uint32 pin)
{
	static char retStr[5];
	memset(retStr, 0, sizeof(retStr));
	strcpy(retStr, "No");

	for (int j = 0; j < 32; j++)
	{
		if (pin & (1 << j))
		{
			sprintf(retStr, "%s%02d", GPIOxToPx(port), j);
			break;
		}
	}

	return retStr;
}

//str = "PA1";"PB2";...
DrvIo* PxxToPortPin(const char* Pxx, uint32* port, uint32* pin)
{
	char ch;
	if (2 == sscanf(Pxx, "P%c%ud", &ch, pin))
	{
		*pin = BIT(*pin);
		*port = ABCDEFG_ToPort(ch);
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		if (g_InIOs[i].periph == *port && g_InIOs[i].pin == *pin) return &g_InIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
	{
		if (g_SwitchIOs[i].periph == *port && g_SwitchIOs[i].pin == *pin) return &g_SwitchIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		if (g_OutputIOs[i].periph == *port && g_OutputIOs[i].pin == *pin) return &g_OutputIOs[i];
	}

	return Null;
}

//����exti�Ż�ȡIO����
DrvIo* IO_GetByExti(uint32 exti)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		if (g_InIOs[i].line == exti) return &g_InIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		if (g_OutputIOs[i].line == exti) return &g_OutputIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
	{
		if (g_SwitchIOs[i].line == exti) return &g_SwitchIOs[i];
	}

	return Null;
}

//����pin�Ż�ȡIO����
DrvIo* IO_Get(IO_ID ioID)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		if (g_InIOs[i].ioID == ioID) return &g_InIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
	{
		if (g_SwitchIOs[i].ioID == ioID) return &g_SwitchIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		if (g_OutputIOs[i].ioID == ioID) return &g_OutputIOs[i];
	}

	return Null;
}

void IO_IRQHandler(DrvIo* p)
{
	uint8 isHigh = PortPin_Read(p);
	if (p->value != isHigh)
	{
		//LOG_TRACE2(CCU_ADDR, CCU_ET_IO_CHANGED, p->ioID, isHigh);
		if (p->stateChangedMsg)
		{
			PostMsgEx(p->stateChangedMsg, p->ioID, isHigh);
		}
		if (p->pOdValue) *p->pOdValue = isHigh;
		p->value = isHigh;

		PFL(DL_IO, "%s(%s)=%d\n", p->desc, PortPinToPxx(p->periph, p->pin), isHigh);
	}
}

#define IO_IRQ_HANDLER(pin)	\
	static DrvIo* p = Null;	\
	if (p == Null)	\
	{	\
		p = IO_Get(pin);	\
	}	\
	if(PortPin_Read(p) != p->value) p->ticks = GET_TICKS();	\
	else p->ticks = 0;	\
	exti_interrupt_flag_clear(p->line);

	/*IO_IRQHandler(p);	\	*/

void EXTI0_IRQHandler(void)  { IO_IRQ_HANDLER(DET_BUCK_OFF); }
void EXTI3_IRQHandler(void)  { IO_IRQ_HANDLER(DET_AC_ON);  }
void EXTI4_IRQHandler(void)  { IO_IRQ_HANDLER(DET_BLE_LOCK); }
void EXTIs_IRQHandler(int fromExti, int toExti)
{
	int ind = 0;
	exti_line_enum linex;

	//�ں������е�һ�γ�ʼ��p[]�����Ż��ж�ִ��ʱ�䣬
	static Bool isFirst = 1;
	static DrvIo* p[11];	//ָ�����DrvIo(EXTI_5...EXTI_15)��
	if (isFirst)	//����ʼ��һ��
	{
		isFirst = 0;
		//��ʼ������ָ��p[11]
		for (int i = 0; i < 11; i++)
		{
			p[i] = IO_GetByExti(BIT(i + 5));
		}
	}

	for (int i = fromExti; i <= toExti; i++)
	{
		linex = (exti_line_enum)BIT(i);
		if (SET == exti_interrupt_flag_get(linex))
		{
			ind = i - 5;
			if (p[ind]) p[ind]->ticks = GET_TICKS();	//IO_IRQHandler(p[ind]);
			if (PortPin_Read(p[ind]) != p[ind]->value) 
				p[ind]->ticks = GET_TICKS();
			else 
				p[ind]->ticks = 0;
			exti_interrupt_flag_clear(linex);
		}
	}
}
void EXTI5_9_IRQHandler(void)   { EXTIs_IRQHandler(5, 9); }
void EXTI10_15_IRQHandler(void) { EXTIs_IRQHandler(10, 15); }

void IO_PinInit(DrvIo* pDrvIo)
{
	/* enable the pin clock */
	rcu_periph_clock_enable(Rcu_Get(pDrvIo->periph));

	//Remap for CTRL_AC5 PB4
	gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP, ENABLE);

	gpio_init(pDrvIo->periph, pDrvIo->mode, GPIO_OSPEED_50MHZ, pDrvIo->pin);

	if(IS_OUT_MODE(pDrvIo->mode))	//Output mode, ����Ĭ��ֵ
	{
		PortPin_Set(pDrvIo->periph, pDrvIo->pin, pDrvIo->value);
	}
	else //if(pDrvIo->mode == GPIO_MODE_IN_FLOATING)
	{
		if (pDrvIo->line) {
			rcu_periph_clock_enable(RCU_AF);
			/* enable and set pin EXTI interrupt to the lowest priority */
			nvic_irq_enable(EXTI_GetIRQn(pDrvIo->line), 2U, 0U);

			/* connect pin EXTI line to pin GPIO pin */
			gpio_exti_source_select(pDrvIo->portSource, pDrvIo->pinSource);

			/* configure pin EXTI line */
			exti_init(pDrvIo->line, EXTI_INTERRUPT, pDrvIo->trig_type);
			exti_interrupt_flag_clear(pDrvIo->line);
		}
		pDrvIo->value = PortPin_Read(pDrvIo);
		if (pDrvIo->pOdValue )*pDrvIo->pOdValue = pDrvIo->value;
	}
}

uint8 PortPin_Read(const DrvIo* pDrvIo)
{
	return gpio_input_bit_get(pDrvIo->periph, pDrvIo->pin);
}

//Pxx = "PA1","PB2"..."PE15"
//value = 0;1
uint8 IODesc_Read(const char* Pxx)
{
	char ch;
	int pin = 0;
	uint8 value = 0;
	if (2 == sscanf(Pxx, "P%c%d", &ch, &pin))
	{
		uint32 port = ABCDEFG_ToPort(ch);
		if (port && pin < 16)
		{
			pin = BIT(pin);
			value = gpio_input_bit_get(port, pin);
			Printf("%s[%X,%X]=%d\n", Pxx, port, pin, value);
		}
		else
		{
			Printf("Port[%d] is invalid\n", port);
		}
	}
	else
	{
		Printf("Param[%s] is invalid\n", Pxx);
	}
	return value;
}

uint8 IO_Read(IO_ID pin)
{
	DrvIo*  p = IO_Get(pin);
	return PortPin_Read(p);
}

//Pxx = "PA1","PB2"..."PE15"
void IODesc_Set(const char* Pxx, uint8 value)
{
	uint32 port, pin = 0;
	DrvIo* p = PxxToPortPin(Pxx, &port, &pin);
	if (p)
	{
		if (IS_OUT_MODE(p->mode))
		{
			if (p->pOdValue)
				* p->pOdValue = value;
			PortPin_Set(port, pin, value);
			Printf("%s[%X,%X]=%d\n", Pxx, port, pin, gpio_input_bit_get(port, pin));
		}
		else
		{
			Printf("%s is in momde\n", Pxx);
		}
	}
	else
	{
		Printf("Param[%s] is invalid\n", Pxx);
	}
}

void IO_Set(IO_ID pin, uint8 value)
{
	DrvIo* p = IO_Get(pin);
	PortPin_Set(p->periph, p->pin, value);
	p->value = PortPin_Read(p);


	//��¼��ǰ���õ�ʱ���
	p->ticks = GET_TICKS();
}


void PortPin_Set(uint32 port, uint32 pin, uint8 value)
{
	Bool isMatch = False;
	static DrvIo* p[3] = { Null, Null, Null };
	if (p[0] == Null) p[0] = IO_Get(CTRL_MCU_LED);
	if (p[1] == Null) p[1] = IO_Get(CTRL_CAN_RED);
	if (p[2] == Null) p[2] = IO_Get(CTRL_CAN_GREEN);
	for (int i = 0; i < 3; i++)
	{
		if (p[i]->periph == port && p[i]->pin == pin)
		{
			isMatch = True;
			break;
		}
	}
	if (!isMatch)
	{
		const char* pStr = PortPinToPxx(port, pin);
		//PFL(DL_IO, "%s set to %d\n", pStr, value);
	}

	if (value)
	{
		GPIO_BOP(port) = pin;
	}
	else
	{
		GPIO_BC(port) = pin;
	}
}

//void IO_IRQEnable(IO_ID io, Bool isEnable)
//{
//	DrvIo* pDrvIo = IO_Get(io);
//	if (pDrvIo)
//	{
//		if (pDrvIo->mode == GPIO_MODE_IN_FLOATING)
//		{
//			pDrvIo->isEnable = isEnable;
//			if (pDrvIo->line)
//			{
//				if (isEnable)
//					exti_interrupt_enable(pDrvIo->line);
//				else
//					exti_interrupt_disable(pDrvIo->line);
//			}
//		}
//	}
//}

void IO_IRQEnable(Bool isEnable)
{
	DrvIo* pDrvIo = g_InIOs;
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, pDrvIo++)
	{
		if (pDrvIo->mode == GPIO_MODE_IN_FLOATING && pDrvIo->line)
		{
			if(isEnable)
				exti_interrupt_enable(pDrvIo->line);
			else
				exti_interrupt_disable(pDrvIo->line);
		}
	}

}

//��ȡ���뿪��ֵ
void IO_ReadSwitchValue()
{
	g_ToggleValue  = IO_Read(DET_SWITCH_BIT3); g_ToggleValue <<= 1;
	g_ToggleValue |= IO_Read(DET_SWITCH_BIT2); g_ToggleValue <<= 1;
	g_ToggleValue |= IO_Read(DET_SWITCH_BIT1); g_ToggleValue <<= 1;
	g_ToggleValue |= IO_Read(DET_SWITCH_BIT0);
	
	//ȡ����ȡ����λ��ֵ
	g_ToggleValue = (~g_ToggleValue) & 0x0F +1;

}

void IO_Stop()
{
	g_isIoStart = False;
	IO_IRQEnable(False);
}

//����IO���󴥷�һ��״̬�ı�	
void IO_AllStateChanged()
{
	DrvIo* p = g_InIOs;

	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
	{
		if (!IS_OUT_MODE(p->mode))
		{
			p->value = 0xFF;
			IO_IRQHandler(p);
		}
	}
}

//����һ��״̬�ı�
void IO_TriggerStateChanged(IO_ID pin)
{
	DrvIo* p = IO_Get(pin);

	p->value = 0xFF;
	IO_IRQHandler(p);
}

void IO_Start()
{
	g_isIoStart = True;
	IO_IRQEnable(True);
}

//���IO״̬���Ƿ����ı�
void IO_CheckIOState()
{
	#define MAX_QUERY_IO_COUNT 5
	static uint8 queryIoCount = 0;
	static DrvIo* pQueryIo[MAX_QUERY_IO_COUNT];

	//��ʼ��pQueryIo������ʼ��һ��
	DrvIo* p = g_InIOs;
	if (queryIoCount == 0)
	{
		for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
		{
			if(p->line == 0)
			{
				Assert(queryIoCount <= MAX_QUERY_IO_COUNT);
				pQueryIo[queryIoCount++] = p;
			}
		}
	}
	else
	{
		//���ڼ��һ��IO��״̬���Ƿ����ı�
		static uint32 initTicks = 0;
		if (initTicks == 0 || SwTimer_isTimerOutEx(initTicks, 10))
		{
			//pAcc1->value PortPin_Read(pAcc1)
			//if(pAcc1->value)
			initTicks = GET_TICKS();
			for (int i = 0; i < queryIoCount; i++, p++)
			{
				p = pQueryIo[i];
				if (PortPin_Read(p) != p->value)
					p->ticks = GET_TICKS();
				else
					p->ticks = 0;
			}
		}
	}

	p = g_InIOs;
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
	{
		//ȥ����
		if (p->ticks && SwTimer_isTimerOutEx(p->ticks, p->ticksDelay))
		{
			p->ticks = 0;
			uint8 isHigh = PortPin_Read(p);
			if (isHigh != p->value)
			{
				IO_IRQHandler(p);
			}
		}
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++)
	{
		IOPair_CheckFault(&g_IOPairs[i]);
	}
}

void IO_Run()
{
	if(!g_isIoStart) return;

	IO_CheckIOState();
}

void IO_Init()
{
	static Obj g_IoObj;
	Obj_Register(&g_IoObj, "IODriver", IO_Start, IO_Stop, IO_Run);

	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		IO_PinInit(&g_InIOs[i]);
		g_InIOs[i].ticksDelay = 500;	//Ĭ�Ϸ���ֵ
		if(g_InIOs[i].ioID == DET_WATER_IN) g_InIOs[i].ticksDelay = 2000;	//�������
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		IO_PinInit(&g_OutputIOs[i]);
		if(g_OutputIOs[i].ioID == CTRL_LCD_POWER) 
			PortPin_Set(g_OutputIOs[i].periph, g_OutputIOs[i].pin, IO_LOW);
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
	{
		IO_PinInit(&g_SwitchIOs[i]);
	}
	IO_ReadSwitchValue();

	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++)
	{
		g_IOPairs[i].pCtrlIo = IO_Get(g_IOPairs[i].ctrlPin);
		g_IOPairs[i].pDetIo = IO_Get(g_IOPairs[i].detPin);
	}
	g_pBucket = IO_Get(DET_BUCKET);
	g_pWaterIn = IO_Get(DET_WATER_IN);
	g_pPumpCtrl = IO_Get(CTRL_PUMP);
	g_pLightCtrl = IO_Get(CTRL_LIGHT);
	g_pLcdResetCtrl = IO_Get(CTRL_LCD_RESET);
}
