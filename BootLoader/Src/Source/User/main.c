
#include "Common.h"
#include "HwFwVer.h"
#include "drv_onchipflash.h"
#include "sysdef.h"
#include "main.h"
#include "drv_systick.h"
#include "SwTimer.h"
#include "drv_spi.h"

const HwFwVer AppInfo={1,0,0,0,5,0};


typedef  void (*iapfun)(void);				//定义一个函数类型的参数.
iapfun jump2app;

//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(uint32_t addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

void JumpToApp(void)
{
    t_APP_INFO* sl_app_info_p;
	uint32_t      crc = 0;

	//判断设备信息是否有效
    sl_app_info_p = (t_APP_INFO*)APP_DEVICE_INFO_START_ADDR;
	
	crc = Crc32_Calc(CRC32_CHECK_CODE,
					(void *)APP_DEVICE_INFO_START_ADDR, 
					(sizeof(t_APP_INFO)-4));
	if(crc == sl_app_info_p->info_crc)
    {
        //设备信息自己的CRC计算正确，设备信息有效
		Printf("app info crc ok.\n");
        crc = 0;
        crc = Crc32_Calc(CRC32_CHECK_CODE,
                        (void *)APPLICATION_START_ADDR, 
                        sl_app_info_p->length);
        
        if(crc == sl_app_info_p->app_crc)
        {
            //APP的CRC计算正确，APP有效
			Printf("Jump to App.\n");
			jump2app=(iapfun)*(__IO uint32_t*)(APPLICATION_START_ADDR+4);		//用户代码区第二个字为程序开始地址(复位地址)		
			MSR_MSP(*(__IO uint32_t*)APPLICATION_START_ADDR);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
			//__set_MSP
			jump2app();									//跳转到APP.
            return ;
        }
		else
		{
			Printf("app crc error!\n");
			NVIC_SystemReset();
		}
    }
	else
	{
		Printf("app info crc error!\n");
		NVIC_SystemReset();
	}
}

void WriteToFlash(void)
{
	t_APP_INFO* sl_app_info_p;
	uint32_t crc = 0;
	uint32_t i = 0;

	//判断设备信息是否有效
    sl_app_info_p = (t_APP_INFO*)APP_DEVICE_INFO_START_ADDR;
	
	crc = Crc32_Calc(CRC32_CHECK_CODE,
					(void *)APP_DEVICE_INFO_START_ADDR, 
					(sizeof(t_APP_INFO)-4));
	if(crc == sl_app_info_p->info_crc)
    {
        //设备信息自己的CRC计算正确，设备信息有效
		Printf("app info crc ok.\n");
		
		if(sl_app_info_p->length>APPLICATION_SIZE)
		{
			Printf("app length too large!\n");
		}
		else
		{
			Printf("erase app flash.\n");
			for(i=0; i< sl_app_info_p->length;)
			{
				gd32_flash_erase(APPLICATION_START_ADDR+i,FMC_PAGE_SIZE);
				i += FMC_PAGE_SIZE;
			}
			uint8_t buff[4];
			for(i=0; i< sl_app_info_p->length;)
			{
				spi_flash_buffer_read(EXTERNFLASH_BIN_START_ADDR+i,buff,4);
				gd32_flash_write(APPLICATION_START_ADDR+4,buff,4);
				i += 4;
			}
			Printf("app length write done.\n");
		}
    }
	else
	{
		Printf("app info crc error!\n");
		NVIC_SystemReset();
	}
	
	
}

int main(void)
{
	uint8_t flash_read_buff[6];
	//通信看门狗TICKS
	uint32 g_commWatchDog = 0;
	
	Crc32_Init();
	gd32_hw_usart_init();
	Printf("\n\nBootloader start!\n");
	HwFwVer_Dump(Null,&AppInfo,Null);
	
//	uint8_t buff[6];
//	memcpy(&buff[0], "update", 6);
//	gd32_flash_erase(APP_DEVICE_INFO_START_ADDR,XMODEM_DATA_SIZE);
//	gd32_flash_write(FLASH_ADDR_UPDATEFLAG_ADDR,buff,6);
	
	gd32_flash_read(FLASH_ADDR_UPDATEFLAG_ADDR,flash_read_buff,6);
	if((flash_read_buff[0] != 'u')||
		(flash_read_buff[1] != 'p')||
		(flash_read_buff[2] != 'd')||
		(flash_read_buff[3] != 'a')||
		(flash_read_buff[4] != 't')||
		(flash_read_buff[5] != 'e'))
	{
		Printf("None update flag.\n");
		JumpToApp();
	}
	Printf("Have update flag.\n");
	//擦除升级标志
    unsigned char vl_buf[XMODEM_DATA_SIZE];

	memcpy(vl_buf, (unsigned char*)APP_DEVICE_INFO_START_ADDR, XMODEM_DATA_SIZE);
	gd32_flash_erase(APP_DEVICE_INFO_START_ADDR,FMC_PAGE_SIZE);
	gd32_flash_write(APP_DEVICE_INFO_START_ADDR,
					(uint8_t *)vl_buf,XMODEM_DATA_SIZE);

	SystemClock_Config();
	spi_flash_init();
	WriteToFlash();
	JumpToApp();
	g_commWatchDog = GET_TICKS();
	while(1)
	{
		if(SwTimer_isTimerOutEx(g_commWatchDog, 3000))
		{
			JumpToApp();
		}
	}
}
