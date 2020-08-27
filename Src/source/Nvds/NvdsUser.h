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

		uint8 Reserved[16];	//����16���ֽ�
		uint8 latestByte;	//�Ӵ洢���������ֽڲ����� EEPROM_LATEST_BYTE��˵���ô洢�����޸ģ��Ѿ�ʧЧ
	}CfgInfo;

	//ϵͳ�������ݶ��󣬴洢һЩϵͳ���й����в�����������Ϣ
	//�ṹ�ֽ�������Ϊż��(2�ֽڶ���)�������޷�д��Flash
	typedef struct
	{
		uint8 firstByte;
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
#ifdef __cplusplus
}
#endif

#endif

