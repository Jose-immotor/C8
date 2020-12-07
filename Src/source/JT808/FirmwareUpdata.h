/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-12     Jose      first version
 */



#ifndef __FIRMWARE_UPDATE__H_
#define __FIRMWARE_UPDATE__H_
 
#ifdef __cplusplus
 extern "C"{
#endif



void EraseFirmwareAllArea(void);
void EraseFirmwareInfoArea(void);
void WriteFirmware(uint32 offset , uint8* pdata , uint16 len);
void ReadFirmware(uint32 offset , uint8* pdata , uint16 len);
Bool CheckFirmware(uint32_t);
void WriteUpdateFlag(void);
void EraseUpdateFlag(void);

/**************************************************************************
**					内部Flash
**				----------------
**				|				|
**				|	boot[24k]	|
**				|---------------|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				|				|
**				-----------------
****************************************************************************/

#define 	XMODEM_DATA_SIZE        128
#define 	BOOTLOADER_SIZE         0x6000UL//应用层//(24kUL*1024UL)
#define 	APP_DEVICE_INFO_SIZE 	1024
#define 	APPLICATION_SIZE  		(128*1024)//100K空间


#define 	FLASH_START_ADDR   0x08000000
#define 	APP_DEVICE_INFO_START_ADDR (FLASH_START_ADDR+BOOTLOADER_SIZE)


#define 	FLASH_ADDR_UPDATEFLAG_ADDR (APP_DEVICE_INFO_START_ADDR+XMODEM_DATA_SIZE)
#define 	APPLICATION_START_ADDR  (APP_DEVICE_INFO_START_ADDR+APP_DEVICE_INFO_SIZE)


#define 	FLASH_START_ADDR   0x08000000
#define		APP_FLASH_ADDR		( FLASH_START_ADDR + BOOTLOADER_SIZE ) 



#pragma pack(1)
// 放在文件尾部
// 文件以128字节为单位,不足补0
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
#pragma pack()



#define 		_APPINFO_ADDR			( (t_APP_INFO*)0x08000000+APPLICATION_SIZE)



#ifdef __cplusplus
}
#endif




#endif //__FIRMWARE_UPDATE__H_






