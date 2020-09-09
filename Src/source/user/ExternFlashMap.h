
/***************************************************
�ⲿFlash�ռ�ӳ�䶨��
***************************************************/

#ifndef _EXTERN_FLASH_H_
#define _EXTERN_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#define  EX_FLASH_SECTOR_SIZE	2048	//�ⲿ������ҳ��СΪ2KB����������С��λ

#define  EX_FLASH_CFG_INFO_ADDR		0
#define  EX_FLASH_CFG_INFO_SIZE		EX_FLASH_SECTOR_SIZE

#define  EX_FLASH_CFG_PDO_INFO_ADDR		(EX_FLASH_CFG_INFO_ADDR + EX_FLASH_CFG_INFO_SIZE)
#define  EX_FLASH_CFG_PDO_INFO_SIZE		EX_FLASH_SECTOR_SIZE

#define  EX_FLASH_CFG_DBG_INFO_ADDR		(EX_FLASH_CFG_PDO_INFO_ADDR + EX_FLASH_CFG_PDO_INFO_SIZE)
#define  EX_FLASH_CFG_DBG_INFO_SIZE		EX_FLASH_SECTOR_SIZE

//��ʼ��ַ��1M
#define	 EX_FLASH_LOG_AREA_ADDR			(EX_FLASH_CFG_DBG_INFO_ADDR + EX_FLASH_CFG_DBG_INFO_SIZE)
#define	 EX_FLASH_LOG_AREA_SIZE			0x100000	//1M

	/*******************************************************************************
	��Flash�ռ�ӳ�䶨��
	*******************************************************************************/
	typedef struct
	{
		uint8 appBackup[EX_FLASH_CFG_INFO_SIZE];		//Ӧ�ó�����, 0-128K
		uint8 keyboardCode[EX_FLASH_CFG_PDO_INFO_SIZE];	//������, 128K-1M
		uint8 reserved[EX_FLASH_CFG_DBG_INFO_SIZE];	//������, 128K-1M
		uint8 logInfo[EX_FLASH_LOG_AREA_SIZE];			//��־, 1M-2M
	}ExFlashMap;

	extern const ExFlashMap* const  g_pExFlashMap;
#ifdef __cplusplus
}
#endif

#endif

