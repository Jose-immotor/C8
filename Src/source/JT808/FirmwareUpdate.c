/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-12     Jose      first version
 */

#include "crc32.h"
#include "drv_spi.h"
#include "ExternFlashMap.h"
#include "FirmwareUpdata.h"
#include "MsgDef.h"
#include "Debug.h"

/*
Page：256 Bytes
Sector：16 Pages（4KB）
Block：16 Sector（64KB
*/


void EraseFirmwareAllArea(void)
{
	uint32 offset = 0 ;
	for( offset = 0 ; offset < EX_FLASH_BIN_AREA_SIZE ; offset += EX_FLASH_SECTOR_SIZE )
	{
		spi_flash_sector_erase( EX_FLASH_BIN_AREA_ADDR + offset );
	}
}

void EraseFirmwareInfoArea(void)
{
	spi_flash_sector_erase( EX_FLASH_BIN_AREA_ADDR );
}


void WriteFirmware(uint32 offset , uint8* pdata , uint16 len)
{
	spi_flash_buffer_write( EX_FLASH_BIN_AREA_ADDR + offset , pdata , len );
}


void ReadFirmware(uint32 offset , uint8* pdata , uint16 len)
{
	spi_flash_buffer_read( EX_FLASH_BIN_AREA_ADDR + offset , pdata , len );	
}


/********************************************************************
**
**
**
**	bin + info
**
**
**
*********************************************************************/

/*
uint32_t app_crc;
uint32_t length;
uint8_t sw_main;
uint8_t sw_s1;
uint8_t sw_s2;
uint8_t sw_build[4];//lsb first
uint8_t hw_main;
uint8_t hw_sub;
uint8_t reserve[107];
uint32_t info_crc;
*/

#define			_FILE_SIZE(s)		( ((s)/128+1) * 128 )

Bool CheckFirmware( uint32_t file_size )
{
	uint8 buff[128] ;
	int32 file_len = 0 , cur_len = 0 , offset = 0 ;
	uint32 crc = 0;
	t_APP_INFO *pAPPInfo = (t_APP_INFO*)buff ;
	uint32_t info_crc = 0 ;
	t_APP_INFO _AppInfo = {0x00};
	Crc32_Init();
	// 先找到最后部分
	file_size = _FILE_SIZE( file_size );
	for( file_len = file_size/*EX_FLASH_BIN_AREA_SIZE*/ - 128 ; file_len > 0 ; file_len -= 128 )
	{
		spi_flash_buffer_read( EX_FLASH_BIN_AREA_ADDR + file_len , buff , 128 );
		crc = Crc32_Calc( CRC32_CHECK_CODE, buff, sizeof(t_APP_INFO)-4 );
		PFL(DL_JT808,"Info[%X] CRC:%x-%x\r\n",file_len,pAPPInfo->info_crc,crc);
		if( crc == pAPPInfo->info_crc )
		{
			memcpy( &_AppInfo , pAPPInfo , sizeof(_AppInfo) );
			PFL(DL_JT808,"Find File Info,APP CRC:%X,Len:%d\r\n",pAPPInfo->app_crc,pAPPInfo->length);
			break ;
		}
	}
	if( file_len <= 0 ) return False ;
	//
	if( file_len > 0 )
	{
		for( crc = CRC32_CHECK_CODE ,offset = 0 ; offset < _AppInfo.length && offset < EX_FLASH_BIN_AREA_SIZE  ; )
		{
			cur_len = _AppInfo.length - offset > 128 ? 128 : _AppInfo.length - offset;
			spi_flash_buffer_read( EX_FLASH_BIN_AREA_ADDR + offset , buff , cur_len );			
			crc = Crc32_Calc( crc, buff, cur_len );
			offset += cur_len ;
			//PFL(DL_JT808,"[%d:%d]:[%02X %02X %02X %02X %02X %02X %02X %02X]\r\n",
			//	offset , cur_len ,
			//	buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7]);
		}
		PFL(DL_JT808,"APP CRC:%X-%X\r\n",crc,_AppInfo.app_crc);
		return crc == _AppInfo.app_crc ;
	}
}



void WriteUpdateFlag(void)
{
	//
}

void EraseUpdateFlag(void)
{
	//
}







