#if 1

#ifndef FM175XX_DRIVER_H
#define FM175XX_DRIVER_H

#include "typedef.h"
#include "Transfer.h"
#include "Bit.h"
#include "SwTimer.h"

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
	FM_STATE_NPD_SET_ANT,
	FM_STATE_WORK,
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
typedef void (*FmFsmFn)(struct _Fm175Drv*);
typedef struct _Fm175Drv
{
	FM17522_STATE state;		//����״̬
	uint8 antPort;				//���ߺ�
	Bool cardIsExist;			//���Ƿ����

	FM17522_CMD cmd;			//������
	TRANSFER_STATUS status;		//����״̬

	const uint8_t* txBuf;		//��������Buffer
	int txBufSize;		//��������Buffer����

	uint8_t* rxBuf;		//��������Buffer
	int rxBufSize;		//�������ݳ���,

	const TransProtocolCfg* cfg;		//����Э������
	TransMgr transMgr;			//�������

	SwTimer timer;				//��������ʱ��ʱ��
	SwTimer delayTimer;			//��������ʱ��ʱ��
	TRANSFER_RESULT latestErr;	//�������������

	unsigned char picc_ats[14];

	FmFsmFn fsm;	//״̬������

	void* obj;	//�û��¼��ص�����
}Fm175Drv;

/***************************************************************
�������ܣ�����������������ᴥ�������¼���ͨ�������¼��ص�����֪ͨ���������ο��¼�����FM_EVENT��
����˵����
	pDrv��ȷ���������
	cmd����������
	txBuf�� ��������ָ��
	txBufSize�� �������ݳ���
	rxBuf��		��������ָ��
	rxBufSize�� �������ݳ���
	timeOutMs�� ��ʱʱ��
	cbObj��		�¼��ص������Ķ������
����ֵ��TRUE-���������ɹ���FALSE-��������ʧ��
***************************************************************/
Bool fm175Drv_transStart(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf
	, int txBufSize
	, void* rxBuf
	, int rxBufSize
	, uint32 timeOutMs
	, void* cbObj
);

inline Bool fm175Drv_isIdle(const Fm175Drv* pDrv) { return pDrv->status == TRANSFER_STATUS_IDLE && pDrv->state == FM_STATE_WORK; };

void fm175Drv_init(Fm175Drv* pDrv, const TransProtocolCfg* cfg, void* evtObj);
void fm175Drv_run(Fm175Drv* pDrv);
void fm175Drv_start(Fm175Drv* pDrv);
void fm175Drv_switchPort(Fm175Drv* pDrv, uint8_t port);

#endif

#endif


