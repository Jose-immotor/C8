#ifndef __NVDS_H_
#define __NVDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "SectorMgr.h"

#define EEPROM_FIRST_BYTE	0x55
#define EEPROM_LATEST_BYTE	0xAA

	/*Nvds�¼�����*/
	typedef enum
	{
		BE_DATA_ERROR,	//�洢�����ݼ���ʧ�ܣ�������Ч
		BE_DATA_OK,		//�洢�����ݼ���ͨ����������Ч
		BE_ON_WRITE_BEFORE,	//д���ݲ���֮ǰ, ������¼�����False����ִ��д����
	}NvdsEventID;

	//�¼���������
	typedef Bool (*NvdsEventFn)(void* pData, NvdsEventID eventId);

	//Nvds�������
	typedef struct _NvdsItem
	{
		//NvdsID
		uint8 id;
		//NvdsID�洢����������
		SectorCfg secCfg;
		//Nnds�����������ָ��
		SectorMgr* sectorMgr;

		//NvdsID�洢���¼���������
		NvdsEventFn Event;

		//secCfg.storage�ľ���������ں�ԭֵ���Ƚϣ������Ƿ�д��FLASH
		//���mirror=Null����ʾд��Flashʱ����ԭֵ�Ƚ�
		void* mirror;
	}NvdsItem;

	typedef struct _Nvds
	{
		const NvdsItem* nvdsItemArray;
		int nvdsItemArrayCount;
	}Nvds;

	//Nvds��ʼ������
	void Nvds_Init(const NvdsItem* nvdsItemArray, int count);
	void Nvds_write(uint8_t nvdsId);

#ifdef __cplusplus
}
#endif

#endif

