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
		uint8 vol;					//����
		uint8 isAccOn:1;			//Զ�̿���ACC  ����ȡ�����������
		uint8 reserved0:7;
		uint8 Reserved[14];	//����15���ֽ�
		//uint8 SN[32];		// SN Jose ���
		uint8 BeaconUUID[16];
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
			uint8 IsForbidDischarge : 1;//BIT[3]:Զ�̶ϵ�/��ֹ�ŵ�
			uint8 IsBatVerifyEn:1;  //BIT[4]:�Ƿ�ʹ�ܵ�������֤
			uint8 IsAlarmMode:1;	///BIT[5]:�Ƿ񾳽�ģʽ/�춯����ʹ��
			uint8 BatVerifyRet:2;	///BIT[6-7]:������У������0�����ԣ���У�顣1���Ϸ���2���Ƿ���			
//			uint8 reserved : 5;		//BIT[3-7]:����
		};
		union
		{
			uint16 isOKstate;
			struct
			{
				uint16 isFlashOk : 1;//BIT[0]:Flash is OK;
				uint16 isGyroOk : 1;//BIT[1]:������ is OK;
				uint16 isNfcOk : 1;	//BIT[2]:NFCͨѶ is OK;
				uint16 isBat0In : 1;//BIT[3]:���0��λ;
				uint16 isBat1In : 1;//BIT[4]:���1��λ;
				uint16 isTakeApart : 1;	//BIT[5]:��
				uint16 isRs485Ok : 1;	//BIT[6]:RS485
				uint16 isCANOk : 1;	//BIT[7]:CAN
				uint16 reserved01 : 5;		//BIT[3-7]:����
			};
		};
		uint32 resetCounter;	//��λ������
		uint32 timeStamp;		//ʱ�����1. SM����ʱ��SM�ı���ʱ��ͬ������ֵ��2. ϵͳ��λʱ��Ҫ�����ֵ
		
		struct
		{
			uint8 isLowPow : 1;//BIT[0]:18650������		
//			uint8 reserved : 5;		//BIT[3-7]:����
		};
		uint8 Reserved[15];		//����15���ֽ�
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

