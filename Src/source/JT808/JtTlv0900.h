
#ifndef  _JT_TLV_0900_H_
#define  _JT_TLV_0900_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "BmsReg.h"

	typedef enum
	{
		TAG_SMART_STATE		= 0x01,	// Smart ������״̬����
		TAG_PMS_STATE		= 0x10,	// PMS ����״̬����

		TAG_BAT_INFO		= 0x20,	//��ػ�����Ϣ
		TAG_BAT_WORK_PARAM	= 0x21,	//��ع�������

		TAG_BAT_TEMP		= 0x22,	// ����¶�����
		TAG_BAT_FAULT		= 0x23,	// ��ع�������
	}CMD0900_TLV;

#define		_SMART_STATE_SIZE		(128)	


#pragma pack(1)	
/*
	Smart �����в��� ��ʽ���£�
	ip,port,hw,fw,url,conmin
*/	
	// PMS ���¶� INT16 (ʵ���¶�+40) + BYTE �����λ״̬
	typedef struct
	{
		int16 pmsTemp;	// �¶�
		uint8 batState;
	}TlvPMSState;

	// ����¶�����
	typedef struct
	{
		uint8	bid[BMS_ID_SIZE];
		uint16	cMostTemp;	// ���MOS�¶�
		uint16	dMostTemp;	// �ŵ�MOS�¶�
		uint16	fuelTemp;	// �������¶�
		uint16	contTemp;	// �������¶�
		uint16	batTemp1;	// ���1�¶�
		uint16	batTemp2;
		uint16	tvsTemp;	// tvs �¶�
		uint16	fuseTemp ;	// ����˿�¶�
	}TlvBatTemp;

	//��ع�������
	typedef struct
	{
		uint8	bid[BMS_ID_SIZE];
		uint16	devft1 ;	// ����1
		uint16	devft2;
		uint16 	opft1 ;
		uint16 	opft2 ;
		uint16 	opwarn1 ;
		uint16	opwarn2 ;
	}TlvBatFault;
	

	typedef struct _TlvBatInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 port;
		uint16 nominalVol;
		uint16 nominalCur;
	}TlvBatInfo;

	typedef struct _TlvBatWorkInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 soc;
		uint16 voltage;
		int16 current;
		uint16 maxCellVol;
		uint16 minCellVol;
		uint8 maxCellNum;
		uint8 minCellNum;

		uint16 maxChgCurr;
		uint16 maxDischgCurr;
		uint16 curWorkFeature;

		uint16 cycCount;
	}TlvBatWorkInfo;

#pragma pack()	

	void JtTlv0900_init();
	int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount);
	void JtTlv0900_updateMirror(const uint8* data, int len);
	void JtTlv0900_updateBatInfo(uint8 port);
	void JtTlv0900_updateStorage(void);

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


