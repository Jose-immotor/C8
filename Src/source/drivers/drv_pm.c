
#include "drv_pm.h"
#include "Common.h"

void Boot(void)
{
	Printf("Mcu reset\n");

	//��λ֮ǰ����
//	Nvds_Write_Setting();
//	Nvds_Write_SysCfg();
	
	rt_thread_mdelay(100);
	
	__set_FAULTMASK(1);      //�ر������ж�
	NVIC_SystemReset();
}
