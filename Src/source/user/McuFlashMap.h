
/***************************************************
MCU内部Flash空间映射定义
***************************************************/

#ifndef __FLASH_MAP_H_
#define __FLASH_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#define MCU_FLASH_SECTOR_SIZE	2048		//MCU的闪存页大小为2KB，擦除的最小单位, 待定。。。
#define BANK0_PAGE_SIZE			MCU_FLASH_SECTOR_SIZE	//bank0的闪存页大小为2KB，擦除的最小单位

	//存储映射，定义本项目的存储器映射区域关系
#define  BOOT_LOADER_ADDR		0
#define  BOOT_LOADER_SIZE		32768

#define  APP_ADDR				(BOOT_LOADER_ADDR + BOOT_LOADER_SIZE)
#define  APP_SIZE				(200*1024)

#define  CFG_INFO_ADDR			(APP_ADDR + APP_SIZE)
#define  CFG_INFO_SIZE			BANK0_PAGE_SIZE

#define  PDO_INFO_ADDR			(CFG_INFO_ADDR + CFG_INFO_SIZE)
#define  PDO_INFO_SIZE			BANK0_PAGE_SIZE

#define  DBG_INFO_ADDR			(PDO_INFO_ADDR + PDO_INFO_SIZE)
#define  DBG_INFO_SIZE			BANK0_PAGE_SIZE


	/*******************************************************************************
	//MCU的数据和代码存储区域映射定义
	*******************************************************************************/
	typedef struct
	{
		uint8 bootLoader[BOOT_LOADER_SIZE];		//Boot loader 代码区, 8K

		uint8 app[APP_SIZE];					//应用程序区, 200K

		uint8 cfgInfo[CFG_INFO_SIZE];			//配置信息区, 2K
		uint8 pdoInfo[PDO_INFO_SIZE];			//过程数据对象信息区, 2K
		uint8 dbgInfo[DBG_INFO_SIZE];			//调试信息区, 2K
	}McuFlashMap;

	extern const McuFlashMap* const  g_pMcuFlashMap;
#ifdef __cplusplus
}
#endif

#endif

