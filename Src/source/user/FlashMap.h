#ifndef __FLASH_MAP_H_
#define __FLASH_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#define BANK0_PAGE_SIZE		2048	//bank0������ҳ��СΪ2KB����������С��λ

	//�洢ӳ�䣬���屾��Ŀ�Ĵ洢��ӳ�������ϵ
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

#define  APP_BACKUP_ADDR		(DBG_INFO_ADDR + DBG_INFO_SIZE)
#define  APP_BACKUP_SIZE		APP_SIZE

#define  BANK0_RESERVED_ADDR		(APP_BACKUP_ADDR + APP_BACKUP_SIZE)
#define  BANK0_RESERVED_SIZE		(0x80000 - BANK0_RESERVED_ADDR)

#define  KEYBOARD_CODE_ADDR		(BANK0_RESERVED_ADDR + BANK0_RESERVED_SIZE)
#define  KEYBOARD_CODE_SIZE		(307200)	//300K

#define	 LOG_AREA_ADDR			(RESERVED_AREA_ADDR + RESERVED_AREA_SIZE)
#define	 LOG_AREA_SIZE			0x100000	//1M

	/*******************************************************************************
	//MCU�����ݺʹ���洢����ӳ�䶨��
	*******************************************************************************/
	typedef struct
	{
		uint8 bootLoader[BOOT_LOADER_SIZE];		//Boot loader ������, 8K

		uint8 app[APP_SIZE];					//Ӧ�ó�����, 200K

		uint8 cfgInfo[CFG_INFO_SIZE];			//������Ϣ��, 2K
		uint8 pdoInfo[PDO_INFO_SIZE];			//�������ݶ�����Ϣ��, 2K
		uint8 dbgInfo[DBG_INFO_SIZE];			//������Ϣ��, 2K

		uint8 appBackUp[APP_BACKUP_SIZE];		//Ӧ�ó���������������, 200K

	}FlashMap;

	extern const FlashMap* const  g_pFlashMap;
#ifdef __cplusplus
}
#endif

#endif

