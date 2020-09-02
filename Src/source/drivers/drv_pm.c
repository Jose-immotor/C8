
#include "drv_pm.h"
#include "Common.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//复位之前保存
//	Nvds_Write_Setting();
//	Nvds_Write_SysCfg();
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //关闭所有中断
	NVIC_SystemReset();
}
