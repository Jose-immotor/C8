#ifndef __NVDS__H_
#define __NVDS__H_

#ifdef __cplusplus
extern "C"{
#endif

//��������NVD(����ʧ������)����Flash�е�λ��
#include "common.h"
#include "typedef.h"
#include "SectorMgr.h"
#include "Battery.h"

#define EEPROM_FIRST_BYTE	0x55
#define EEPROM_LATEST_BYTE	0xAA

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
	uint8  version; //�汾�ţ�������ڵ�һ���ֽ�,����뱣��İ汾�Ų�һ�£��ᵼ��ɾ����������ݶ�ʹ��Ĭ��ֵ��
	uint32 debugLevel;	
	uint8  timeIsValid:1;	//resetTime,ֵ�Ƿ���Ч
	uint8  isLogValid:1;	//readLogStartSector��postLogInd�Ƿ���Ч
	uint8  IsUnPostLog:1;
	uint8  reserved1:5;
	uint32 resetTime;			//��λʱ��
	uint8  resetReason;			//��λ����
	uint8  readLogStartSector;	//����ʼ����ƫ��
	uint32 postLogInd;			//�ϴ�Log�ı��
	uint32 readLogCount;		//��ȡ��¼��
	uint8  reserved2[8];		//����
	uint8  SerialNums[BAT_SERIALNUM_COUNT][2];//[SERIAL_NUM_SIZE];
}SysCfg;

//������Ϣ������,�����ڽṹ������firstByte��latestByte,����У���Flash�ж�ȡ���������ݵ�������
//�ṹ�ֽ�������Ϊż��(2�ֽڶ���)�������޷�д��Flash
typedef struct
{
	uint8 firstByte;
	uint32 debugLevel;		//������Ϣ�����־
	uint8 pmsAddrForPrintf;	//ָ����ӡ�����Pms��ַ��0����Чֵ,��ӡ���е�PMS�ڵ����Ϣ��
	uint8 pad;				//��λ�ֽ�
	uint8 Reserved[32];		//����32���ֽ�
	uint8 latestByte;		//�Ӵ洢���������ֽڲ����� EEPROM_LATEST_BYTE��˵���ô洢�����޸ģ��Ѿ�ʧЧ
}DebugInfo;

#pragma pack() 

extern SysCfg	g_SysCfg;
extern DebugInfo	*g_pDbgInfo;


#define MAX_LOG_SECTOR 8
	
#define MAX_FILE_NUM 2
#define FILE_BUF_SIZE (64*1024)
//Flash�Ŀռ�ӳ��
typedef struct _NvdsMap
{
	uint8	miscAddr[SECTOR_SIZE];		//����Misc����
	uint8	runSettingAddr[SECTOR_SIZE];//����Setting����
	//moveTrackAddr[0]���汸�����ݣ�moveTrackAddr[1]�����������, ÿ��moveTrackAddr[1]д��֮�󣬾Ͱ������Ƶ�������
	uint8	runCfgAddr[SECTOR_SIZE];	
	uint8	binBackupCfg[SECTOR_SIZE];		//Smart�̼�������������Ϣ
	
	uint8	reserved[FILE_BUF_SIZE-4*SECTOR_SIZE];	//�����հף�64K����BIN�ļ�

	//�ļ��ռ䶨��
	uint8	binFileBuf[FILE_BUF_SIZE];		//BIN�ļ�
	
	//PMS�ļ��ռ䶨��
	uint8	pmsFileBuf[FILE_BUF_SIZE];		//PMS BIN�ļ�
	
	//BMS�ļ��ռ䶨��
	uint8	bmsFileBuf[FILE_BUF_SIZE];		//PMS BIN�ļ�
	
	#ifdef CFG_LOG
	//LOG���ݿռ䶨��
	uint8	logBuf[MAX_LOG_SECTOR][SECTOR_SIZE];		//LOG�����ļ�
	uint8	reserved1[FILE_BUF_SIZE-MAX_LOG_SECTOR*SECTOR_SIZE];	//�����հף�64K����BIN�ļ�
	#endif
	
	uint8	binBuf1[FILE_BUF_SIZE];		//BIN�ļ�
	uint8	binBuf2[FILE_BUF_SIZE];		//BIN�ļ�
}NvdsMap;

typedef enum
{
	NVDS_VER_INFO = 0,
	NVDS_CFG_INFO,
	NVDS_PDO_INFO,
	NVDS_DBG_INFO,
}NvdsID;

/*Nvds�¼�����*/
typedef enum
{
	BE_DATA_ERROR,	//�洢�����ݼ���ʧ�ܣ�������Ч
	BE_DATA_OK,		//�洢�����ݼ���ͨ����������Ч
	BE_ON_WRITE_BEFORE,	//д���ݲ���֮ǰ
}BlockEventID;
//�¼���������
typedef void (*BlockEventFn)(void* pData, BlockEventID eventId);
typedef struct
{
	NvdsID		id;					//����
	SectorMgr	sectorMgr;			//��������			
	BlockEventFn Event;	//���洢����Ϣ��Чʱ������Ĭ��ֵ�ĺ���
}Nvds;


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

