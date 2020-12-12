
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

static t_APP_INFO gAPP_Info = {0x00};
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(uint32_t addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

#if 0
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

#else

int main(void)
{
	//通信看门狗TICKS
	uint32 g_commWatchDog = 0;
	uint8_t i = 0 ;
	uint32_t cur_ms = 0;
	Crc32_Init();
	gd32_hw_usart_init();
	Printf("\n\nBootloader start!\n");
	HwFwVer_Dump(Null,&AppInfo,Null);

	SystemClock_Config();
	spi_flash_init();

	// 先检测外部是否有要更新的服务器
	if( IsExFlashAppUpdate(EX_FLASH_BIN_AREA_ADDR,EX_FLASH_BIN_AREA_SIZE) )
	{
		Printf("Find Exflash Firmware,Update APP & Check APP...\r\n");

		for( i = 0 ; i < 3 ; i++ )
		{
			UpdateFirmware( EX_FLASH_BIN_AREA_ADDR , APP_FLASH_ADDR , gAPP_Info.length );

			if( CheckAppIsOK( APP_FLASH_ADDR , gAPP_Info.length , gAPP_Info.app_crc ) )
			{
				DelExFlashAppFlag( EX_FLASH_BIN_AREA_ADDR );
				break ;
			}
			else
			{
				Printf("Check InFlash APP Error\r\n");
				DelInFlashAppFlag( APP_FLASH_ADDR );
				//system_ms_tick
				cur_ms = system_ms_tick ;
				while( system_ms_tick - cur_ms < 100 );
			}
		}
	}
	else
	{
		Printf("Exflash No APP Update\r\n");
	}
	//
	if( _IS_APP_STACK( *(__IO uint32*)APP_FLASH_ADDR ) )
	{
		Printf("Jump to App.0x%X\r\n",APP_FLASH_ADDR );		
		jump2app=(iapfun)*(__IO uint32_t*)(APP_FLASH_ADDR+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		__disable_irq() ;
		//MSR_MSP(*(__IO uint32_t*)APP_FLASH_ADDR);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		__set_MSP(*(__IO uint32_t*)APP_FLASH_ADDR);
		jump2app();									//跳转到APP.
        return ;
	}
	else
	{
		Printf("InFlash No APP[%X]\r\n",*(__IO uint32*)APP_FLASH_ADDR);
	}
	while(1);
}






#if 0

Bool IsExFlashAppUpdate( uint32_t exflash_addr , uint32_t size )
{
	uint8_t buff[128] = {0x00};
	int32_t pos = 0 , cur_len = 0;
	uint32_t crc = 0 ;
	t_APP_INFO *pAPP_Info = (t_APP_INFO*)buff ;
	//
	for( pos = 0 , crc = CRC32_CHECK_CODE ; pos < size ; pos += 128 )
	{
		spi_flash_buffer_read( exflash_addr/*EX_FLASH_BIN_AREA_ADDR*/ + pos , buff , sizeof(buff) );
		crc = Crc32_Calc( CRC32_CHECK_CODE, buff, sizeof(t_APP_INFO) - 4 );
		Printf("ExFlash:[%X][%02X %02X %02X %02X %02X %02X %02X %02X ] %x:%x\r\n",
			exflash_addr + pos ,
			buff[0],buff[1],buff[2],buff[3],
			buff[4],buff[5],buff[6],buff[7],
			crc,pAPP_Info->info_crc);
	}
}

#else

Bool IsExFlashAppUpdate( uint32_t exflash_addr , uint32_t size )
{
	uint8_t buff[128] = {0x00};
	int32_t pos = 0 , cur_len = 0;
	uint32_t crc = 0 ;
	t_APP_INFO *pAPP_Info = (t_APP_INFO*)buff ;
	
	// 先获取头,如果为正常标识则为升级文件
	spi_flash_buffer_read( exflash_addr/*EX_FLASH_BIN_AREA_ADDR*/ + 0 , buff , sizeof(buff) );

	Printf("ExFlash:[%02X %02X %02X %02X]\r\n",buff[0],buff[2],buff[3],buff[4]);
	
	if( ! _IS_APP_STACK(*(uint32_t *)buff) ) return False ;
	// 文件的Info
	for( pos = size/*EX_FLASH_BIN_AREA_SIZE*/ - 128 ; pos > 0 ; pos -= 128 )
	{
		spi_flash_buffer_read( exflash_addr/*EX_FLASH_BIN_AREA_ADDR*/ + pos , buff , 128 );
		crc = Crc32_Calc( CRC32_CHECK_CODE, buff, sizeof(t_APP_INFO)-4 );
		if( crc == pAPP_Info->info_crc ) break ;
	}
	if( pos < 0 ) return False ;
	memcpy( &gAPP_Info , pAPP_Info , sizeof(gAPP_Info) );
	Printf("Find ExFlash APP:%X-%X-%X-%X\r\n",
		gAPP_Info.app_crc,gAPP_Info.info_crc,gAPP_Info.length,pos );
	// 文件的完整性
	for( crc = CRC32_CHECK_CODE ,pos = 0 ; pos < gAPP_Info.length && pos < size/*EX_FLASH_BIN_AREA_SIZE*/  ; )
	{
		cur_len = gAPP_Info.length - pos > 128 ? 128 : gAPP_Info.length - pos;
		spi_flash_buffer_read( exflash_addr/*EX_FLASH_BIN_AREA_ADDR*/ + pos , buff , cur_len );			
		crc = Crc32_Calc( crc, buff, cur_len );
		pos += cur_len ;
		//Printf("%x:%X %X %X %X %X %X %X %X\r\n",
		//	cur_len,buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7]);
	}
	Printf("CRC:%X-%X\r\n",crc,gAPP_Info.app_crc);
	return crc == gAPP_Info.app_crc ;	
}
#endif //

static void DelExFlashAppFlag(uint32_t exflash_addr )
{
	spi_flash_sector_erase( exflash_addr );
}


Bool UpdateFirmware(  uint32_t exflash_addr , uint32_t inflash_addr, uint32_t firmware_size )
{
	uint32_t offset = 0 ;
	uint8_t buff[128] = {0x00};
	for( offset = 0 ; offset < firmware_size ; offset += 128 )
	{
		spi_flash_buffer_read( exflash_addr/*EX_FLASH_BIN_AREA_ADDR*/ + offset , buff , 128 );
		if( (inflash_addr + offset) % FMC_PAGE_SIZE  == 0 )
		{
			gd32_flash_erase( inflash_addr + offset ,FMC_PAGE_SIZE);
		}
		gd32_flash_write( inflash_addr + offset , buff , 128 );
	}
}

static void DelInFlashAppFlag(uint32_t inflash_addr )
{
	gd32_flash_erase( inflash_addr ,FMC_PAGE_SIZE);
}


static Bool CheckAppIsOK( uint32_t inflash_addr , uint32_t app_size , uint32_t app_crc )
{
	uint8_t buff[128] = {0x00};
	int32_t offset = 0 , cur_len = 0;
	uint32_t crc = 0 ;
	for( offset = 0 , crc = CRC32_CHECK_CODE ; offset < app_size ;  )
	{
		cur_len = app_size - offset > 128 ? 128 : app_size - offset;
		gd32_flash_read( inflash_addr + offset , buff , cur_len ) ;
		crc = Crc32_Calc( crc, buff, cur_len );
		offset += cur_len ;
	}
	return crc == app_crc ;
}


static void ExInFlash_Test( uint32_t ex_flash , uint32_t in_flash , uint32_t size)
{
	uint8_t ex_buff[128] = {0x00};
	uint8_t in_buff[128] = {0x00};
	uint32_t offset = 0 ;
	uint16_t cur_len = 0 ;
	//
	for( offset = 0 ; offset < size ; )
	{
		cur_len = size - offset > 128 ? 128 : size - offset ;
		gd32_flash_read( in_flash + offset , in_buff , cur_len ) ;
		spi_flash_buffer_read( ex_flash + offset , ex_buff , cur_len );
		if( 0 != memcmp( in_buff , ex_buff , cur_len) )
		{
			Printf("ExIn Error:%x\r\n", offset );
			return ;
		}
		offset += cur_len ;
	}

	Printf("ExIn Check Ok\r\n");
}



#endif





