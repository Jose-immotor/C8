/**************************************************************
ϵͳ���浽Flash�����ݶ��壺
	������ÿ���洢�ṹ������firstByte��latestByte,����У���Flash�ж�ȡ���������ݵ������ԡ�
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
		NVDS_CFG_INFO = 1,	//������Ϣ������ID
		NVDS_PDO_INFO,		//��������������ID
		NVDS_DBG_INFO,		//������Ϣ������ID
	}NvdsID;

	//������Ϣ������,�洢ϵͳ�����й����������겻�޸Ļ��ߺ����޸ĵ����ݣ�
	//�ṹ�ֽ�������Ϊż��(2�ֽڶ���)�������޷�д��Flash
	typedef struct
	{
		uint8 firstByte;
		struct
		{
			uint8 isActive : 1;		//BIT[0]:�����Ƿ񼤻�
			uint8 reserved : 7;		//BIT[1-7]:����
		};
		uint8 Reserved[16];	//����16���ֽ�
		uint8 latestByte;	//�Ӵ洢���������ֽڲ����� EEPROM_LATEST_BYTE��˵���ô洢�����޸ģ��Ѿ�ʧЧ
	}CfgInfo;

	//ϵͳ�������ݶ��󣬴洢һЩϵͳ���й����в�����������Ϣ
	//�ṹ�ֽ�������Ϊż��(2�ֽڶ���)�������޷�д��Flash
	typedef struct
	{
		uint8 firstByte;
		struct
		{
			uint8 isRemoteAccOn : 1;//BIT[0]:����Զ�̵��
			uint8 isWheelLock : 1;	//BIT[1]:�����
			uint8 isCanbinLock : 1;	//BIT[2]:������
			uint8 reserved : 5;		//BIT[3-7]:����
		};

		uint32 resetCounter;	//��λ������
		uint32 timeStamp;		//ʱ�����1. SM����ʱ��SM�ı���ʱ��ͬ������ֵ��2. ϵͳ��λʱ��Ҫ�����ֵ
		uint8 Reserved[16];		//����16���ֽ�
		uint8 latestByte;		//�Ӵ洢���������ֽڲ����� EEPROM_LATEST_BYTE��˵���ô洢�����޸ģ��Ѿ�ʧЧ
	}PdoInfo;

	//������Ϣ������,�����ڽṹ������firstByte��latestByte,����У���Flash�ж�ȡ���������ݵ�������
	//�ṹ�ֽ�������Ϊż��(2�ֽڶ���)�������޷�д��Flash
	typedef struct
	{
		uint8 firstByte;
		uint32 debugLevel;		//������Ϣ�����־
		uint8 Reserved[16];		//����16���ֽ�
		uint8 latestByte;		//�Ӵ洢���������ֽڲ����� EEPROM_LATEST_BYTE��˵���ô洢�����޸ģ��Ѿ�ʧЧ
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

