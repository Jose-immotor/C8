
#include "Common.h"
#include "drv_gpio.h"
#include <rtthread.h>
#include "Led.h"
#include "RcuMap.h"

static Bool g_isIoStart = False;
//uint8 g_ToggleValue = 0;

//DrvIo* g_pPumpCtrl = Null;
//DrvIo* g_pBucket;
//DrvIo* g_pWaterIn;
//DrvIo* g_pLightCtrl = Null;
//DrvIo* g_pLcdResetCtrl = Null;

//所有命名包含“ON”表示高电平有效, 包含“OFF”表示低电平有效
static DrvIo g_InIOs[] =
{
	{IO_NFC_IRQ_A, "NFC_IRQ_A", GPIOE, GPIO_PIN_15, GPIO_MODE_IN_FLOATING},//, EXTI_15, 
		//GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_15, EXTI_TRIG_RISING}
	{IO_GRYO_IRQ, "GYRO_IRQ", GPIOB, GPIO_PIN_8, GPIO_MODE_IN_FLOATING, EXTI_8, 
		GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8, EXTI_TRIG_BOTH},
	{IO_GPRS_INSERT, "GPRS_INSERT", GPIOA, GPIO_PIN_2, GPIO_MODE_IN_FLOATING, EXTI_2, 
		GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_2, EXTI_TRIG_BOTH},
};

//所有命名包含“ON”表示高电平有效, 包含“OFF”表示低电平有效
static DrvIo g_OutputIOs[] =
{
	//输出配置	
	{CTRL_MCU_LED	, "CTRL_MCU_LED"	, GPIOE, GPIO_PIN_2 ,GPIO_MODE_OUT_PP},
	{IO_NFC_NPD_A	, "NFC_NPD_A"		, GPIOE, GPIO_PIN_14 ,GPIO_MODE_OUT_PP},
	{IO_BOOST_EN	, "BOOST_EN"		, GPIOE, GPIO_PIN_11 ,GPIO_MODE_OUT_PP},
	{IO_NFC_PWR_OFF	, "NFC_PWR_OFF"		, GPIOE, GPIO_PIN_0 ,GPIO_MODE_OUT_PP},
};

////=============================================
////控制IO和反馈IO状态匹配定义,
//typedef struct
//{
//	IO_ID ctrlPin;		//控制IO
//	uint8 ctrlPinState;	//控制IO状态

//	IO_ID detPin;		//反馈IO
//	uint8 detPinState;	//反馈IO状态

//	MSG_ID msgId;		//如果发现ctrlPinState和detPinState的状态不一致，则发送消息

//	DrvIo* pCtrlIo;		//指向控制IO对象
//	DrvIo* pDetIo;		//指向反馈IO对象

//	uint32 ticks;		//用于检测状态IO延时
//	uint8 ctlPinCurrentVal;	//ctlPin当前值,用于判定是否发生改变
//	uint8 detPinCurrentVal;	//detPin当前值,用于判定是否发生改变
//	Bool  isFault;		//是否有错误
//}IOPair;

//static IOPair g_IOPairs[] =
//{
//	{DET_AC_ON		  , IO_HIGH, DET_12V_1	    , IO_LOW, MSG_IO_FAULT},
//	{DET_AC_ON		  , IO_HIGH, DET_12V_2	    , IO_LOW, MSG_IO_FAULT},
//	{CTRL_AC3_PMS	  , IO_HIGH, DET_12V_3	    , IO_LOW, MSG_IO_FAULT},
//	{CTRL_AC3_PMS	  , IO_HIGH, DET_12V_4	    , IO_LOW, MSG_IO_FAULT},
//	{CTRL_18650BP_ON  , IO_HIGH, DET_12V_18650BP, IO_LOW, MSG_IO_FAULT},
//	{CTRL_12VBP_ON	  , IO_HIGH, DET_12V_BP	    , IO_LOW, MSG_IO_FAULT},
//	{CTRL_PUMP		  , IO_HIGH, DET_PUMP_OFF   , IO_LOW, MSG_IO_FAULT},
//};

//void IO_IRQHandler(DrvIo* p);

//const IOPair* IOPair_Get(IO_ID pin)
//{
//	const IOPair* p = g_IOPairs;
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++, p++)
//	{
//		if (p->ctrlPin == pin) return p;
//	}
//	return Null;
//}

//void IO_Dump(DrvIo* p, const char* head)
//{
//	if (head == Null) head = "";
//	Printf("%s%s-%s: %d\n", head, PortPinToPxx(p->periph, p->pin), p->desc, PortPin_Read(p));
//}

//void IOs_Dump()
//{
//	Bool isGoIn = False;
//	Printf("IO Dump:\n")
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
//	{
//		IO_Dump(&g_InIOs[i], "\t");
//		//Printf("\t%s-%s: \t%d\n", PortPinToPxx(g_InIOs[i].periph, g_InIOs[i].pin), g_InIOs[i].desc, g_InIOs[i].value);
//	}
//	Printf("\n");
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
//	{
//		IO_Dump(&g_OutputIOs[i], "\t");
//		//Printf("\t%s-%s: \t%d\n", PortPinToPxx(g_OutputIOs[i].periph, g_OutputIOs[i].pin), g_OutputIOs[i].desc, g_OutputIOs[i].value);
//	}

//	Printf("IOs Fault Dump:\n")
//	const IOPair* p = g_IOPairs;
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++, p++)
//	{
//		if (p->isFault)
//		{
//			isGoIn = True;
//			IO_Dump(p->pCtrlIo, "\t");
//			IO_Dump(p->pDetIo, "\t");
//		}
//	}
//	if (!isGoIn)
//	{
//		Printf("No fault\n");
//	}
//}

//void IOPair_CheckFault(IOPair* pPair)
//{
//	DrvIo* pCtrlIo = pPair->pCtrlIo;
//	DrvIo* pDet = pPair->pDetIo;
//	Bool isCheck = False;

//	//对于输出模式的IO脚，需要延时检测。
//	//对于输入模式的IO脚，不需要延时检测，不能在这里了修改ticks和value值，否则会影响输入IO的防抖判断逻辑。
//	if (IS_OUT_MODE(pCtrlIo->mode))
//	{
//		//如果发现控制IO或者检测IO的状态发生变化，则延时检查2个IO的状态是否匹配
//		//如果不匹配，说明发生故障
//		if (pPair->ctlPinCurrentVal != PortPin_Read(pCtrlIo)
//			|| pPair->detPinCurrentVal != PortPin_Read(pDet))
//		{
//			pCtrlIo->ticks = GET_TICKS();
//			pPair->ctlPinCurrentVal = PortPin_Read(pCtrlIo);
//			pCtrlIo->value = PortPin_Read(pCtrlIo);
//		}
//		if (pPair->detPinCurrentVal != PortPin_Read(pDet))
//		{
//			pCtrlIo->ticks = GET_TICKS();
//			pPair->detPinCurrentVal = PortPin_Read(pDet);
//		}
//		isCheck = (pCtrlIo->ticks && SwTimer_isTimerOutEx(pCtrlIo->ticks, 500));
//		if(isCheck) pCtrlIo->ticks = 0;
//	}
//	else
//	{
//		//如果CtrlIo值变化或者DetIo值变化，才能执行下面检测代码。
//		if (pPair->ctlPinCurrentVal != PortPin_Read(pCtrlIo)
//			|| pPair->detPinCurrentVal != PortPin_Read(pDet))
//		{
//			isCheck = True;
//			pPair->ctlPinCurrentVal = PortPin_Read(pCtrlIo);
//			pPair->detPinCurrentVal = PortPin_Read(pDet);
//		}
//	}

//	//延时500ms检测
//	if (isCheck)
//	{
//		IO_IRQHandler(pDet);
//		if (pPair->ctrlPinState == pPair->detPinState)
//		{
//			//判断设置和反馈是否匹配
//			pPair->isFault = (pDet->value != pCtrlIo->value);
//		}
//		else
//		{
//			//判断设置和反馈是否匹配
//			pPair->isFault = (pDet->value == pCtrlIo->value);
//		}

//		if (pPair->isFault)
//		{
//			LOG_TRACE1(CCU_ADDR, CCU_CATID_IO, pDet->ioID, CCU_ET_IO_DET_FAULT, pDet->value);
//			PostMsgEx(pPair->msgId, pPair->ctrlPin, pPair->detPin);
//			PFL(DL_MAIN, "%s is fault, \n", pDet->desc);
//		}
//	}
//}
//===========================================================================
//perith和名称匹配定义
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

////根据port和pin参数，转换为字符串，例如 PortNameToStr(GPIOA, GPIO_PIN_3) -> "PA3"
//返回值："PA0";"PA1"..."PG15"
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

//	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
//	{
//		if (g_SwitchIOs[i].periph == *port && g_SwitchIOs[i].pin == *pin) return &g_SwitchIOs[i];
//	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		if (g_OutputIOs[i].periph == *port && g_OutputIOs[i].pin == *pin) return &g_OutputIOs[i];
	}

	return Null;
}

////根据exti号获取IO对象
//DrvIo* IO_GetByExti(uint32 exti)
//{
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
//	{
//		if (g_InIOs[i].line == exti) return &g_InIOs[i];
//	}

//	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
//	{
//		if (g_OutputIOs[i].line == exti) return &g_OutputIOs[i];
//	}

//	for (int i = 0; i < GET_ELEMENT_COUNT(g_SwitchIOs); i++)
//	{
//		if (g_SwitchIOs[i].line == exti) return &g_SwitchIOs[i];
//	}

//	return Null;
//}

//根据pin号获取IO对象
DrvIo* IO_Get(IO_ID ioID)
{
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		if (g_InIOs[i].ioID == ioID) return &g_InIOs[i];
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		if (g_OutputIOs[i].ioID == ioID) return &g_OutputIOs[i];
	}

	return Null;
}

//void IO_IRQHandler(DrvIo* p)
//{
//	uint8 isHigh = PortPin_Read(p);
//	if (p->value != isHigh)
//	{
//		//LOG_TRACE2(CCU_ADDR, CCU_ET_IO_CHANGED, p->ioID, isHigh);
//		if (p->stateChangedMsg)
//		{
//			PostMsgEx(p->stateChangedMsg, p->ioID, isHigh);
//		}
//		if (p->pOdValue) *p->pOdValue = isHigh;
//		p->value = isHigh;

//		PFL(DL_IO, "%s(%s)=%d\n", p->desc, PortPinToPxx(p->periph, p->pin), isHigh);
//	}
//}

//#define IO_IRQ_HANDLER(pin)	\
//	static DrvIo* p = Null;	\
//	if (p == Null)	\
//	{	\
//		p = IO_Get(pin);	\
//	}	\
//	if(PortPin_Read(p) != p->value) p->ticks = GET_TICKS();	\
//	else p->ticks = 0;	\
//	exti_interrupt_flag_clear(p->line);

//	/*IO_IRQHandler(p);	\	*/

//void EXTI0_IRQHandler(void)  { IO_IRQ_HANDLER(DET_BUCK_OFF); }
//void EXTI3_IRQHandler(void)  { IO_IRQ_HANDLER(DET_AC_ON);  }
//void EXTI4_IRQHandler(void)  { IO_IRQ_HANDLER(DET_BLE_LOCK); }
//void EXTIs_IRQHandler(int fromExti, int toExti)
//{
//	int ind = 0;
//	exti_line_enum linex;

//	//在函数运行第一次初始化p[]对象，优化中断执行时间，
//	static Bool isFirst = 1;
//	static DrvIo* p[11];	//指向对象DrvIo(EXTI_5...EXTI_15)，
//	if (isFirst)	//仅初始化一次
//	{
//		isFirst = 0;
//		//初始化对象指针p[11]
//		for (int i = 0; i < 11; i++)
//		{
//			p[i] = IO_GetByExti(BIT(i + 5));
//		}
//	}

//	for (int i = fromExti; i <= toExti; i++)
//	{
//		linex = (exti_line_enum)BIT(i);
//		if (SET == exti_interrupt_flag_get(linex))
//		{
//			ind = i - 5;
//			if (p[ind]) p[ind]->ticks = GET_TICKS();	//IO_IRQHandler(p[ind]);
//			if (PortPin_Read(p[ind]) != p[ind]->value) 
//				p[ind]->ticks = GET_TICKS();
//			else 
//				p[ind]->ticks = 0;
//			exti_interrupt_flag_clear(linex);
//		}
//	}
//}
//void EXTI5_9_IRQHandler(void)   { EXTIs_IRQHandler(5, 9); }
//void EXTI10_15_IRQHandler(void) { EXTIs_IRQHandler(10, 15); }

void IO_PinInit(DrvIo* pDrvIo)
{
	/* enable the pin clock */
	rcu_periph_clock_enable(Rcu_Get(pDrvIo->periph));

	gpio_init(pDrvIo->periph, pDrvIo->mode, GPIO_OSPEED_50MHZ, pDrvIo->pin);

	if(IS_OUT_MODE(pDrvIo->mode))	//Output mode, 设置默认值
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

////Pxx = "PA1","PB2"..."PE15"
////value = 0;1
//uint8 IODesc_Read(const char* Pxx)
//{
//	char ch;
//	int pin = 0;
//	uint8 value = 0;
//	if (2 == sscanf(Pxx, "P%c%d", &ch, &pin))
//	{
//		uint32 port = ABCDEFG_ToPort(ch);
//		if (port && pin < 16)
//		{
//			pin = BIT(pin);
//			value = gpio_input_bit_get(port, pin);
//			Printf("%s[%X,%X]=%d\n", Pxx, port, pin, value);
//		}
//		else
//		{
//			Printf("Port[%d] is invalid\n", port);
//		}
//	}
//	else
//	{
//		Printf("Param[%s] is invalid\n", Pxx);
//	}
//	return value;
//}

//uint8 IO_Read(IO_ID pin)
//{
//	DrvIo*  p = IO_Get(pin);
//	return PortPin_Read(p);
//}

////Pxx = "PA1","PB2"..."PE15"
//void IODesc_Set(const char* Pxx, uint8 value)
//{
//	uint32 port, pin = 0;
//	DrvIo* p = PxxToPortPin(Pxx, &port, &pin);
//	if (p)
//	{
//		if (IS_OUT_MODE(p->mode))
//		{
//			if (p->pOdValue)
//				* p->pOdValue = value;
//			PortPin_Set(port, pin, value);
//			Printf("%s[%X,%X]=%d\n", Pxx, port, pin, gpio_input_bit_get(port, pin));
//		}
//		else
//		{
//			Printf("%s is in momde\n", Pxx);
//		}
//	}
//	else
//	{
//		Printf("Param[%s] is invalid\n", Pxx);
//	}
//}

//void IO_Set(IO_ID pin, uint8 value)
//{
//	DrvIo* p = IO_Get(pin);
//	PortPin_Set(p->periph, p->pin, value);
//	p->value = PortPin_Read(p);


//	//记录当前设置的时间戳
//	p->ticks = GET_TICKS();
//}


void PortPin_Set(uint32 port, uint32 pin, uint8 value)
{
	Bool isMatch = False;
	static DrvIo* p[3] = { Null, Null, Null };
	if (p[0] == Null) p[0] = IO_Get(CTRL_MCU_LED);
	if (p[1] == Null) p[1] = IO_Get(CTRL_MCU_LED);
	if (p[2] == Null) p[2] = IO_Get(CTRL_MCU_LED);
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

////void IO_IRQEnable(IO_ID io, Bool isEnable)
////{
////	DrvIo* pDrvIo = IO_Get(io);
////	if (pDrvIo)
////	{
////		if (pDrvIo->mode == GPIO_MODE_IN_FLOATING)
////		{
////			pDrvIo->isEnable = isEnable;
////			if (pDrvIo->line)
////			{
////				if (isEnable)
////					exti_interrupt_enable(pDrvIo->line);
////				else
////					exti_interrupt_disable(pDrvIo->line);
////			}
////		}
////	}
////}

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

////获取拨码开关值
//void IO_ReadSwitchValue()
//{
//	g_ToggleValue  = IO_Read(DET_SWITCH_BIT3); g_ToggleValue <<= 1;
//	g_ToggleValue |= IO_Read(DET_SWITCH_BIT2); g_ToggleValue <<= 1;
//	g_ToggleValue |= IO_Read(DET_SWITCH_BIT1); g_ToggleValue <<= 1;
//	g_ToggleValue |= IO_Read(DET_SWITCH_BIT0);
//	
//	//取反后取低四位的值
//	g_ToggleValue = (~g_ToggleValue) & 0x0F +1;

//}

//外置模块插入，拔出处理
void gprs_insert(void)
{
	rt_interrupt_enter();

	if(g_isPowerDown)
	{
		SetWakeUpType(WAKEUP_GPRS_INSERT);
	}
	
//	if(GPIO_READ(PB, 14))
//	{
//		PostMsg(MSG_GYRO_ASSERT);
//	}
	
	rt_interrupt_leave();
}

void IO_Stop()
{
	g_isIoStart = False;
	IO_IRQEnable(False);
}

////所有IO对象触发一次状态改变	
//void IO_AllStateChanged()
//{
//	DrvIo* p = g_InIOs;

//	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
//	{
//		if (!IS_OUT_MODE(p->mode))
//		{
//			p->value = 0xFF;
//			IO_IRQHandler(p);
//		}
//	}
//}

////触发一次状态改变
//void IO_TriggerStateChanged(IO_ID pin)
//{
//	DrvIo* p = IO_Get(pin);

//	p->value = 0xFF;
//	IO_IRQHandler(p);
//}

void IO_Start()
{
	g_isIoStart = True;
	IO_IRQEnable(True);
}

////检查IO状态，是否发生改变
//void IO_CheckIOState()
//{
//	#define MAX_QUERY_IO_COUNT 5
//	static uint8 queryIoCount = 0;
//	static DrvIo* pQueryIo[MAX_QUERY_IO_COUNT];

//	//初始化pQueryIo，仅初始化一次
//	DrvIo* p = g_InIOs;
//	if (queryIoCount == 0)
//	{
//		for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
//		{
//			if(p->line == 0)
//			{
//				Assert(queryIoCount <= MAX_QUERY_IO_COUNT);
//				pQueryIo[queryIoCount++] = p;
//			}
//		}
//	}
//	else
//	{
//		//周期检查一下IO的状态，是否发生改变
//		static uint32 initTicks = 0;
//		if (initTicks == 0 || SwTimer_isTimerOutEx(initTicks, 10))
//		{
//			//pAcc1->value PortPin_Read(pAcc1)
//			//if(pAcc1->value)
//			initTicks = GET_TICKS();
//			for (int i = 0; i < queryIoCount; i++, p++)
//			{
//				p = pQueryIo[i];
//				if (PortPin_Read(p) != p->value)
//					p->ticks = GET_TICKS();
//				else
//					p->ticks = 0;
//			}
//		}
//	}

//	p = g_InIOs;
//	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++, p++)
//	{
//		//去抖动
//		if (p->ticks && SwTimer_isTimerOutEx(p->ticks, p->ticksDelay))
//		{
//			p->ticks = 0;
//			uint8 isHigh = PortPin_Read(p);
//			if (isHigh != p->value)
//			{
//				IO_IRQHandler(p);
//			}
//		}
//	}

//	for (int i = 0; i < GET_ELEMENT_COUNT(g_IOPairs); i++)
//	{
//		IOPair_CheckFault(&g_IOPairs[i]);
//	}
//}

void IO_Run()
{
//	if(!g_isIoStart) return;

//	IO_CheckIOState();
}
static DrvIo* g_pBootEnIO = Null;
int IO_Init(void)
{
	static const Obj obj = {"IODriver", IO_Start, IO_Stop, IO_Run};
	ObjList_add(&obj);
	
	for (int i = 0; i < GET_ELEMENT_COUNT(g_InIOs); i++)
	{
		IO_PinInit(&g_InIOs[i]);
		g_InIOs[i].ticksDelay = 500;	//默认防抖值
	}

	for (int i = 0; i < GET_ELEMENT_COUNT(g_OutputIOs); i++)
	{
		IO_PinInit(&g_OutputIOs[i]);
	}
	
	g_pBootEnIO = IO_Get(IO_BOOST_EN);
	PortPin_Set(g_pBootEnIO->periph, g_pBootEnIO->pin, True);
	return 0;
}
//INIT_BOARD_EXPORT(IO_Init);
