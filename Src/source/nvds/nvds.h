#ifndef __NVDS__H_
#define __NVDS__H_

#ifdef __cplusplus
extern "C"{
#endif

//定义所有NVD(非易失性数据)的在Flash中的位置
#include "common.h"
#include "typedef.h"

#include "Battery.h"
	
#define    BLOCK_SIZE       0x10000     // 64K Block size
#define    SECTOR_SIZE      0x1000      // 4K SectorMgr size
#define    PAGE_SIZE        0x0100      // 256 Byte Page size
#define    BLOCK_NUM        (FlashSize / BLOCK_SIZE)	
	
#pragma pack(1) 

#define SYS_CFG_VER 6
#define DATATIME_VALID 1
#define BAT_SERIALNUM_COUNT 3
typedef struct _SysCfg
{
	uint8  version; //版本号，必须放在第一个字节,如果与保存的版本号不一致，会导致删除保存的数据而使用默认值。
	uint32 debugLevel;	
	uint8  timeIsValid:1;	//resetTime,值是否有效
	uint8  isLogValid:1;	//readLogStartSector，postLogInd是否有效
	uint8  IsUnPostLog:1;
	uint8  reserved1:5;
	uint32 resetTime;			//复位时间
	uint8  resetReason;			//复位理由
	uint8  readLogStartSector;	//读起始扇区偏移
	uint32 postLogInd;			//上传Log的编号
	uint32 readLogCount;		//读取记录数
	uint8  reserved2[8];		//保留
	uint8  SerialNums[BAT_SERIALNUM_COUNT][SERIAL_NUM_SIZE];
}SysCfg;
#pragma pack() 

extern SysCfg	g_SysCfg;

#define MAX_LOG_SECTOR 8
	
#define MAX_FILE_NUM 2
#define FILE_BUF_SIZE (64*1024)
//Flash的空间映射
typedef struct _NvdsMap
{
	uint8	miscAddr[SECTOR_SIZE];		//保存Misc内容
	uint8	runSettingAddr[SECTOR_SIZE];//保存Setting内容
	//moveTrackAddr[0]保存备份内容，moveTrackAddr[1]保存更新内容, 每当moveTrackAddr[1]写满之后，就把数据移到备份区
	uint8	runCfgAddr[SECTOR_SIZE];	
	uint8	binBackupCfg[SECTOR_SIZE];		//Smart固件备份区配置信息
	
	uint8	reserved[FILE_BUF_SIZE-4*SECTOR_SIZE];	//保留空白，64K对齐BIN文件

	//文件空间定义
	uint8	binFileBuf[FILE_BUF_SIZE];		//BIN文件
	
	//PMS文件空间定义
	uint8	pmsFileBuf[FILE_BUF_SIZE];		//PMS BIN文件
	
	//BMS文件空间定义
	uint8	bmsFileBuf[FILE_BUF_SIZE];		//PMS BIN文件
	
	#ifdef CFG_LOG
	//LOG数据空间定义
	uint8	logBuf[MAX_LOG_SECTOR][SECTOR_SIZE];		//LOG数据文件
	uint8	reserved1[FILE_BUF_SIZE-MAX_LOG_SECTOR*SECTOR_SIZE];	//保留空白，64K对齐BIN文件
	#endif
	
	uint8	binBuf1[FILE_BUF_SIZE];		//BIN文件
	uint8	binBuf2[FILE_BUF_SIZE];		//BIN文件
}NvdsMap;

extern const NvdsMap* g_pNvdsMap;

void Nvds_HwInit(void);
void Nvds_Start(void);
Bool Nvds_Write_Misc(void);
Bool Nvds_Write_Setting(void);
Bool Nvds_Write_SysCfg(void);
#define Nvds_Write_Md5Key() Nvds_Write_Misc() 
void Nvds_Run(void);
void Nvds_Set(uint8 ind, uint32 value, uint8 isWrite);
void Nvds_SysCfgSetDefault(void);
void Nvds_WriteTotalTrip(void);
void Nvds_Reset(void);
void Nvds_SettingDefault(void);
//void Nvds_InitSectorMgr(SectorMgr* pSector, uint32 startAddr, void* pItem, uint16 itemSize, uint8 version);
void Nvds_Tester(void);
void NvdsMap_Dump(void);
void SysCfg_Dump(void);

int env_nvds_init(void);
#ifdef __cplusplus
}
#endif

#endif

