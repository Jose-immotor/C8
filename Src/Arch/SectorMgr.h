#ifndef __SECTOR_MGR__H_
#define __SECTOR_MGR__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
/******************************************************************/
#define MAX_ITEM_SIZE 	128		//���Ԫ���ֽ���

	typedef Bool(*ItemVerifyFn)(const void* pItem, int len);
	typedef Bool(*FmcReadfun)(uint32 addr, void* buf, int len);
	typedef Bool(*FmcWritefun)(uint32 addr, const void* pData, int len);
	typedef void(*FmcErasefun)(uint32 addr, int len);

/******************************************************************/

typedef struct _SectorCfg
{
	uint32 startAddr;		//���Գ�ʼ��������д��������ʼ��ַ
	uint16 sectorSize;		//���Գ�ʼ��������������С��

	void*  storage;			//���Գ�ʼ�������������Flash��ȡ���������ݣ���д�뵽Flash�е����ݡ�
	uint16 storageSize;		//���Գ�ʼ��������storage��С

	void* exchangeBuf;		//storage�Ľ��������������ȱ������storageSize
	uint16 exchangeBufSize;	//��������������

	//Ԫ����Ч�Լ��麯������������ʼ��ʱ��Flash��ȡ���ݣ��ж��Ƿ���ϵ�ǰ����Flash�洢�����ݿ���ʱ�ɰ汾д��ģ�
	//�������ʧ�ܣ������еĴ洢���ݱ���Ϊ����Ч�ģ����ᱻ��������
	ItemVerifyFn Verify;	

	FmcReadfun  Read;	//Read data from flash
	FmcWritefun Write;	//Write data into flash
	FmcErasefun	Erase;	//Erase data in flash
}SectorCfg;

typedef struct _Sector
{
	uint32 writeOffset;	//�����ڵ�дƫ�Ƶ�ַ
	uint32 readOffset;	//�����ڵĶ�ƫ�Ƶ�ַ
	uint32 itemCount;		//��Ч��Item����

	const SectorCfg* cfg;
}SectorMgr;

Bool SectorMgr_init(SectorMgr* pSector, const SectorCfg* cfg);
Bool SectorMgr_WriteEx(SectorMgr* pSector, const void* pData);
Bool SectorMgr_Write(SectorMgr* pSector);
int SectorMgr_Read(SectorMgr* pSector, void* pBuff);
void SectorMgr_CalcOffset(SectorMgr* pSector);
Bool SectorMgr_isFull(SectorMgr* pSector);
Bool SectorMgr_isEmpty(SectorMgr* pSector);
Bool SectorMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff);
void SectorMgr_Erase(SectorMgr* pSector);
Bool SectorMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc, uint8* pageBuf, int bufLen);
void SectorMgr_Dump(const SectorMgr* pSectorMgr);
void SectorMgr_Reset(SectorMgr* pSector);
Bool SectorMgr_IsChanged(const SectorMgr* pSector);

#ifdef __cplusplus
}
#endif

#endif

