
#include "Gyro.h"
#include "mir3da.h"
#include "common.h"
#include "drv_i2c.h"


Gyro g_Gyro;

//陀螺仪中断处理函数
static uint32 g_GyroIsrTicks = 0;
void Gyro_Isr()
{
	rt_interrupt_enter();

	if(g_isPowerDown)
	{
		SetWakeUpType(WAKEUP_GYRO);
	}
	
	g_GyroIsrTicks = GET_TICKS();

	if((Pms_GetStatus() == PMS_DEEP_SLEEP)||(Pms_GetStatus() == PMS_ACC_OFF))
		Pms_postMsg(PmsMsg_GyroIrq, 0, 0);
//	if(GPIO_READ(PB, 14))
//	{
//		PostMsg(MSG_GYRO_ASSERT);
//	}
	g_pdoInfo.isGyroOk  = True;
//	g_Settings.isGyroAssert = True;
	
	PFL(DL_GYRO,"gyro interrupt!\n");
	
	rt_interrupt_leave();
}

Bool Gyro_isIdle(uint32 second)
{
	return SwTimer_isTimerOutEx(g_GyroIsrTicks, second * 1000);
}

void Gyro_Run()
{
}

void Gyro_Stop()
{
//	if(!g_Gyro.m_isPowerOn) return;
//	//Printf("Gyro_Stop()\n");
////	GYRO_DISABLE();
////	GYRO_POWER_OFF();
//	g_Gyro.m_isPowerOn = False;
}

void Gyro_Start()
{
	if(g_Gyro.m_isPowerOn) return;
	//Printf("Gyro_Start()\n");
	
	g_Gyro.m_isPowerOn = True;
//	GPIO_SetMode(GYRO_PWR_PORT, GYRO_PWR_PIN, GPIO_PMD_OUTPUT);
//	GYRO_POWER_ON();

	//等待上电稳定
	rt_thread_mdelay(200);	
	rt_hw_i2c_init(GYRO_I2C);

	g_pdoInfo.isGyroOk = (mir3da_init(FS_2G, 0x1B) != -1);	//FS_8G
	//g_Settings.isGyroOk = (mir3da_init(FS_2G, 0x08) != -1);	//FS_8G不够灵敏
//	g_pdoInfo.isGyroOk = (mir3da_init(FS_2G, 0x06) != -1);
	//g_Settings.isGyroOk = (mir3da_init(FS_2G, 0x04) != -1);	//FS_8G太灵敏
	mir3da_open_interrupt(0);
}

void Gyro_Init()
{
	const static Obj obj = {
	.name = "Gyro",
	.Start = Gyro_Start,
	.Run = Null,//Gyro_Run,
	.Stop = Gyro_Stop,
	};

	ObjList_add(&obj);

	memset(&g_Gyro, 0, sizeof(g_Gyro));
		
//	Mcu_RegGpioIsr(GYRO_INT_PORT, GYRO_INT_PIN, Gyro_Isr);
	g_Gyro.m_isPowerOn = False;
}

