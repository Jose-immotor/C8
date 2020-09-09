/**************************************************************
系统保存到Flash的内容定义：
	必须在每个存储结构中增加firstByte和latestByte,用于校验从Flash中读取的区块内容的完整性。
**************************************************************/

#ifndef __NVDS_USER_H_
#define __NVDS_USER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Nvds.h"

	#define NVDS_ITEM_COUNT 3

	typedef enum
	{
		NVDS_CFG_INFO = 1,	//配置信息区内容ID
		NVDS_PDO_INFO,		//过程数据区内容ID
		NVDS_DBG_INFO,		//调试信息区内容ID
	}NvdsID;

	//配置信息区内容,存储系统在运行过程中配置完不修改或者很少修改的内容，
	//结构字节数必须为偶数(2字节对齐)，否则无法写入Flash
	typedef struct
	{
		uint8 firstByte;
		struct
		{
			uint8 isActive : 1;		//BIT[0]:车辆是否激活
			uint8 reserved : 7;		//BIT[1-7]:保留
		};
		uint8 Reserved[16];	//保留16个字节
		uint8 latestByte;	//从存储区读出的字节不等于 EEPROM_LATEST_BYTE，说明该存储区被修改，已经失效
	}CfgInfo;

	//系统过程数据对象，存储一些系统运行过程中产生的数据信息
	//结构字节数必须为偶数(2字节对齐)，否则无法写入Flash
	typedef struct
	{
		uint8 firstByte;
		struct
		{
			uint8 isRemoteAccOn : 1;//BIT[0]:车辆远程点火
			uint8 isWheelLock : 1;	//BIT[1]:轮毂锁
			uint8 isCanbinLock : 1;	//BIT[2]:座舱锁
			uint8 reserved : 5;		//BIT[3-7]:保留
		};

		uint32 resetCounter;	//复位计数器
		uint32 timeStamp;		//时间戳，1. SM启动时会SM的本地时间同步到该值。2. 系统复位时，要保存该值
		uint8 Reserved[16];		//保留16个字节
		uint8 latestByte;		//从存储区读出的字节不等于 EEPROM_LATEST_BYTE，说明该存储区被修改，已经失效
	}PdoInfo;

	//调试信息区内容,必须在结构中增加firstByte和latestByte,用于校验从Flash中读取的区块内容的完整性
	//结构字节数必须为偶数(2字节对齐)，否则无法写入Flash
	typedef struct
	{
		uint8 firstByte;
		uint32 debugLevel;		//调试信息输出标志
		uint8 Reserved[16];		//保留16个字节
		uint8 latestByte;		//从存储区读出的字节不等于 EEPROM_LATEST_BYTE，说明该存储区被修改，已经失效
	}DbgInfo;

	void NvdsUser_Write(NvdsID id);
	void NvdsUser_Init();

	extern CfgInfo	g_cfgInfo;
	extern PdoInfo	g_pdoInfo;
	extern DbgInfo	g_degInfo;
	
	extern const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT];
#ifdef __cplusplus
}
#endif

#endif

