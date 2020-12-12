
#ifndef __MAIN_H__
#define __MAIN_H__

#include "Common.h"

#pragma pack(push)
#pragma pack(1)
typedef struct
{
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
}t_APP_INFO;
#pragma pack(pop)



static Bool IsExFlashAppUpdate( uint32_t exflash_addr , uint32_t size );	// 外部是否有升级的Flash
static Bool UpdateFirmware(  uint32_t exflash_addr , uint32_t inflash_addr, uint32_t firmware_size );
static void DelExFlashAppFlag(uint32_t exflash_addr );
static void DelInFlashAppFlag(uint32_t inflash_addr );
static Bool CheckAppIsOK( uint32_t inflash_addr , uint32_t app_size , uint32_t app_crc );


static void ExFlash_Test(void);
static void ExInFlash_Test( uint32_t , uint32_t ,uint32_t size);

#define		_IS_APP_STACK(x)		(  ( (x) & 0x2FFE0000 ) == 0x20000000 )





#endif
