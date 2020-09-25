#if 1

#ifndef FM175XX_DRIVER_H
#define FM175XX_DRIVER_H

#include "typedef.h"
#include "Transfer.h"
#include "Bit.h"
#include "SwTimer.h"
#include "fm175Reg.h"

typedef enum _NFC_ISO_TYPE
{
	NFC_ISO_TYPE_A = 0,	//ISO14443_A
	NFC_ISO_TYPE_B = 1,	//ISO14443_B
}NFC_ISO_TYPE;

typedef enum _FM17522_MSG
{
	FM_MSG_RUN,
	FM_MSG_SWITCH_NFC,	//Fsm(FM_MSG_SWITCH_NFC, iicAddr)
}FM17522_MSG;

//FM17522���������붨��
typedef enum _FM17522_CMD
{
	FM_CMD_IDLE			=0x00 ,
	FM_CMD_MEM			=0x01 ,
	FM_CMD_RANDOMID		=0x02 ,
	FM_CMD_CALCCRC		=0x03 ,
	FM_CMD_TRANSMIT		=0x04 ,
	FM_CMD_NOCMDCHANGE	=0x07 ,
	FM_CMD_RECEIVE		=0x08 ,
	FM_CMD_TRANSCEIVE	=0x0C ,
	FM_CMD_MFAUTHENT	=0x0E ,
	FM_CMD_SOFTRESET	=0x0F ,
}FM17522_CMD;

//����״̬����
typedef enum _FM17522_STATE
{
	FM_STATE_INIT = 0x01,
	FM_STATE_NPD_LOW,
	FM_STATE_NPD_HIGH,
	FM_STATE_SEARCH_CARD,	//�ѿ�״̬
	FM_STATE_SLEEP,			//���ߣ��͹���
	FM_STATE_TRANSFER,		//�ѿ��ɹ�,����״̬
}FM17522_STATE;

#define REG_IRQ_MASK_TX			BIT_6
#define REG_IRQ_MASK_RX			BIT_5
#define REG_IRQ_MASK_IDLE		BIT_4
#define REG_IRQ_MASK_HI_ALERT	BIT_3
#define REG_IRQ_MASK_LO_ALERT	BIT_2
#define REG_IRQ_MASK_ERR		BIT_1
#define REG_IRQ_MASK_TIMEOUT	BIT_0

struct _Fm175Drv;
typedef void (*FmIrqFn)(struct _Fm175Drv* pDrv);

typedef struct _REG_CommIRq
{
	uint8 Set1 : 1;			//BIT[7]
	uint8 TxDoneIrq : 1;	//BIT[6]
	uint8 RxDoneIrq : 1;	//BIT[5]
	uint8 IdleIrq : 1;		//BIT[4]
	uint8 HiAlertIrq : 1;	//BIT[3]
	uint8 LoAlertIrq : 1;	//BIT[2]
	uint8 ErrIrq : 1;		//BIT[1]
	uint8 TimeOutIrq : 1;	//BIT[0]
}REG_CommIRq;

struct _Fm175Drv;
typedef void (*FmFsmFn)(struct _Fm175Drv* p, FM17522_MSG msg, uint32 param);
typedef struct _Fm175Drv
{
	FM17522_STATE state;		//����״̬
	uint8 antPort;				//���ߺ�
	Bool cardIsExist;			//���Ƿ����

	FM17522_CMD cmd;				//������
	TRANSFER_STATUS transStatus;	//����״̬

	const uint8_t* txBuf;		//��������Buffer
	int txBufSize;		//��������Buffer����

	uint8_t* rxBuf;		//��������Buffer
	int rxBufSize;		//�������ݳ���,

	/*************************************************
	��������ʱ����� FIFO �������ռ�ʣ�µ��ֽ���С�ڵ��� WaterLevel ������ֽ����� ����HiAlert�ж�
	��������ʱ����� FIFO �е��ֽ���С�ڵ��� WaterLevel �ֽ����� ����LoAlert�ж�
	*************************************************/
	uint8 waterLevel;

	const TransProtocolCfg* cfg;//����Э������
	TransMgr transMgr;			//�������

	SwTimer sleepWdTimer;		//˯�߿��Ź���ʱ���������ι������ʱ�����˯��
	SwTimer timer;				//��������ʱ��ʱ��

	TRANSFER_RESULT latestErr;	//�������������

	IICReg iicReg;	//IIC�Ĵ�������

	FmFsmFn fsm;	//״̬������

	void* obj;				//�û��¼��ص���������
	TransEventFn	Event;	//�û��¼��ص�����ָ��
}Fm175Drv;

/***************************************************************
�������ܣ�����������������ᴥ�������¼���ͨ�������¼��ص�����֪ͨ���������ο��¼�����FM_EVENT��
����˵����
	pDrv��ȷ���������
	txBuf�� ��������ָ��
	txBufSize�� �������ݳ���
	rxBuf��		��������ָ��
	rxBufSize�� �������ݳ���
	cbObj��		�¼��ص������Ķ������
����ֵ��TRUE-�����ʼ���ɹ���FALSE-�����ʼ��ʧ��
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj);

Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
);

inline Bool fm175Drv_isIdle(const Fm175Drv* pDrv) { return pDrv->transStatus == TRANSFER_STATUS_IDLE; };

void fm175Drv_init(Fm175Drv* pDrv, uint8 iicAddr, const TransProtocolCfg* cfg, TransEventFn	Event);
void fm175Drv_run(Fm175Drv* pDrv);
void fm175Drv_start(Fm175Drv* pDrv);
Bool fm175Drv_switchNfc(Fm175Drv* pDrv, uint8 iicAddr);
void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param);

#endif

#endif


