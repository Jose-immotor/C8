
/***************************************************
外部Flash空间映射定义
***************************************************/

#ifndef _EXTERN_FLASH_H_
#define _EXTERN_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#define EX_FLASH_SECTOR_SIZE	512	//外部的闪存页大小为2KB，擦除的最小单位

#define  APP_BACKUP_ADDR		0
#define  APP_BACKUP_SIZE		(128*1024)

#define  KEYBOARD_CODE_ADDR		(APP_BACKUP_ADDR + APP_BACKUP_SIZE)
#define  KEYBOARD_CODE_SIZE		(64*1024)	//64K

#define  EX_FLASH_RESERVED_ADDR		(KEYBOARD_CODE_ADDR + KEYBOARD_CODE_SIZE)
#define  EX_FLASH_RESERVED_SIZE		(1024*1024 - APP_BACKUP_SIZE)

//起始地址在1M
#define	 LOG_AREA_ADDR			(EX_FLASH_RESERVED_ADDR + EX_FLASH_RESERVED_SIZE)
#define	 LOG_AREA_SIZE			0x100000	//1M

	/*******************************************************************************
	部Flash空间映射定义
	*******************************************************************************/
	typedef struct
	{
		uint8 appBackup[APP_BACKUP_SIZE];		//应用程序区, 0-128K
		uint8 keyboardCode[KEYBOARD_CODE_SIZE];	//保留区, 128K-1M
		uint8 reserved[EX_FLASH_RESERVED_SIZE];	//保留区, 128K-1M
		uint8 logInfo[LOG_AREA_SIZE];			//日志, 1M-2M
	}ExFlashMap;

	extern const ExFlashMap* const  g_pExFlashMap;
#ifdef __cplusplus
}
#endif

#endif

