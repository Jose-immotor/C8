
/***************************************************
MCU�ڲ�Flash�ռ�ӳ�䶨��
***************************************************/

#ifndef __FLASH_MAP_H_
#define __FLASH_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#define MCU_FLASH_SECTOR_SIZE	2048		//MCU������ҳ��СΪ2KB����������С��λ, ����������
#define BANK0_PAGE_SIZE			MCU_FLASH_SECTOR_SIZE	//bank0������ҳ��СΪ2KB����������С��λ

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
	}McuFlashMap;

	extern const McuFlashMap* const  g_pMcuFlashMap;
#ifdef __cplusplus
}
#endif

#endif

