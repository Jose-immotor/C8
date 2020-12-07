//=======================================Copyright(c)===========================================
// 								  ���������Ƽ����޹�˾
//---------------------------------------�ļ���Ϣ----------------------------------------------
//�ļ���   	: MBFindMap.h
//������  	: Handry
//��������	: 
//����	    : Modbus�ӻ���ַӳ�䴦��ͷ�ļ�
//-----------------------------------------------��ǰ�汾�޶�----------------------------------
//�޸���   	: Allen
//�汾	    :
//�޸�����	:
//����	    :
//=============================================================================================

#ifndef _MB_FINDMAP_H
#define _MB_FINDMAP_H

#include "typedef.h"
//=============================================================================================
//�궨��
#define BMS_ID_SIZE     6

//=============================================================================================

typedef struct _BmsRegCtrl
{
	uint16_t reserved0 : 8;  //BIT[8-15]:����
	uint16_t chgEn : 1;  //BIT[0]:���ʹ��
	uint16_t dischgEn : 1;  //BIT[1]:�ŵ�ʹ��
	uint16_t preDischg : 1;  //BIT[2]:Ԥ����ʹ��
	uint16_t reserved1 : 2;  //BIT[3-4]:����
	uint16_t sleepEn : 1;  //BIT[5]:ǳ����ʹ��
	uint16_t deepSleepEn : 1; //BIT[6]:��˯��ʹ��
	uint16_t chgrIn : 1;  //BIT[7]:���������
	
}BmsRegCtrl;

// 1�ֽڶ���
#pragma pack(1)

//BMS�豸��Ϣ�Ĵ���
typedef struct
{
	/*
	Addr 0 ~ 49
	*/
	uint16_t prver;		// Э��汾��
	uint16_t hwver;		// Ӳ���汾��
	uint16_t blver;		// Boot �汾��
	uint16_t fwmsv;		// �̼��汾---���ΰ汾��
	uint16_t fwrev;		// �̼��汾---�����汾��
	uint16_t fwbnh;		// �̼��汾---����汾����
	uint16_t fwbnl;		// �̼��汾---����汾����
	uint16_t mcun12;	// MCU ID 
	uint16_t mcun34;
	uint16_t mcun56;
	uint16_t mcun78;
	uint16_t btype;		// �������
	uint16_t bvolt;		// ���ѹ
	uint16_t bcap;		// �����
	uint16_t sn12;		// ���к�
	uint16_t sn34;
	uint16_t sn56;
	uint16_t sn78;
	uint16_t erech;		// �����¼�� ����
	uint16_t erecl;		// �����¼�� ����
	uint16_t lrech;		// �����¼�� ����
	uint16_t lrecl;		// �����¼�� ����
	uint16_t ltsta;		// ��ʷ״̬
	uint16_t ltmaxv;	// ��ʷ��ߵ����ѹ
	uint16_t ltmaxvn;	// ��ʷ��ߵ����ѹ��� 
	uint16_t ltminv;	// ��ʷ��͵����ѹ
	uint16_t ltminvn;	// ��ʷ��͵����ѹ���
	uint16_t ltmaxt;	// ��ʷ����¶�
	uint16_t ltmaxtn;	// ��ʷ����¶ȱ��
	uint16_t ltmint;	// ��ʷ����¶�
	uint16_t ltmintn;	// ��ʷ����¶ȱ��
	uint16_t ltmaxcc;	// ��ʷ��������
	uint16_t ltmaxdc;	// ��ʷ���ŵ����
	uint16_t df_ver;	// DF�汾
	// 34 - 49 Ԥ��
}BmsReg_info;

//BMSֻ���Ĵ���
typedef struct
{
	/*
	Addr:256 ~ 363
	*/
	uint16_t userId[4];		//BID
	/*
		Bit0 : CHG_MOS : 1 :���MOS��,0:���MOS�ر�
		Bit1 ��DIS_MOS ��1���ŵ�MOS��,0:�ŵ�MOS�ر�
		Bit2 : PRE_CHG_MOS : 1 Ԥ�ŵ�MOS��,0���ر�
		Bit3 : G2_SAT : 1:G2��Ч��0:G2��Ч
		Bit4 : BALANCE_STA : 1:��������ھ���()
		Bit5 : DEV_FAULT: 1:�豸����
		Bit6 : OP_FAULT 1:���й���
		Bit7 : OP_WARNING 1:���и澯
		Bit8 : CERTIFI : BMS��֤HOST 1����Ҫ��֤��0����֤ͨ��
		Bit9 : DISASSEN : 1:������Ч,0:������Ч
		Bit10 : EXTERN MODULE :1:����ģ����λ��0������ģ�鲻��λ
		Bit11 ~ Bit13 : ����
		BIt14 : CHG_FULL 1:�����
		Bit15 :����
	*/
	uint16_t state;			//STATE BMS ״̬
	uint16_t soc;			//SOC	0.1 ��ؼ��ɵ�״̬
	uint16_t tvolt;			//�ܵ�ѹ	
	uint16_t tcurr;			//�ܵ���,�ŵ�Ϊ��,���Ϊ��
	uint16_t htemp;			//����¶�
	uint16_t htnum;			//����¶ȱ��
	uint16_t ltemp;			//����¶�
	uint16_t ltnum;			//����¶ȱ��
	uint16_t hvolt;			//��ߵ����ѹ
	uint16_t hvnum;			//��ߵ����ѹ���
	uint16_t lvolt;			//��͵����ѹ
	uint16_t lvnum;			//��͵����ѹ��� 
	uint16_t dsop;			//10s�����ŵ����
	uint16_t csop;			//10s����������
	uint16_t soh;			//SOH
	uint16_t cycle;			//ѭ������
	uint16_t rcap;			//ʣ������ 
	uint16_t fcap;			//��������
	uint16_t fctime;		//����ʱ��
	uint16_t prow;			//ʣ������
	uint16_t devft1;		//�豸����1
	uint16_t devft2;		//�豸����2
	uint16_t opft1;			//���й���1
	uint16_t opft2;			//���й���2
	uint16_t opwarn1;		//���и澯1
	uint16_t opwarn2;		//���и澯2
	uint16_t cmost;			//���MOS�¶�
	uint16_t dmost;			//�ŵ�MOS�¶�
	uint16_t pret;			//�������¶�
	uint16_t cont;			//�������¶�
	uint16_t btemp[2];		//�¶�1&2
	uint16_t bvolt[16];		//�����ѹ
	uint16_t balasta;		// ����ֵ
	//uint16_t ignore[6];		// ���ٶ�x,y,z,MCU3.3,Ԥ�ŵ���ؼ��,�����Ƶ���
	//uint16_t maxcvolt;		//������ѹ
	//uint16_t pretvolt;		//�����Ƶ�ѹ
	//uint16_t tvstemp;		//TVS���¶�
	//uint16_t battempsp1;	//���1�����ٶ�
	//uint16_t battempsp2;	//���2�����ٶ�
	//uint16_t pretempsp;		//�����������ٶ�
	//uint16_t 					//��о��ѹ����½��ٶ�
	//�����������
	//������ŵ����
	//����˿�¶�
	//Curr_DET_T Bit3-Bit0:��ŵ��������ʱ��,Bit9~Bit4:10s���ŵ����,Bit15~bit10:10s��ƽ������
	// 327~338 Ԥ��
	// 339~342 ����ģ������
	// 343 -- �����17��ѹ
	// 344 -- AFE��������ѹ
	// 235 -- AFE����PACK�˵�ѹ
	// 246 -- ��17��ؾ���״̬
	// 247 -- ������ SOC
	// 248 -- ������ SOH
	// 249 -- ������--ѭ������
	// 250 -- ������--ʣ������
	// 251 -- ������--��������
	// 252 -- SOX ��ʷ���������λ(mAH)
	// 253 -- SOX ��ʷ���������λ(mAH)
	// 254 -- SOX ��ʷ�ŵ�������λ(mAH)
	// 255 -- SOX ��ʷ�ŵ�������λ(mAH)
	// 256 -- ��Χģ��״̬--- Bit0 :1��λ,0δ��λ,Bit1:0δ���ӷ�����,1�����ӷ�����,Bit2:0�޶���,1�ж���
	// 257 - 358 :����
	// 359 - 360 :ΰ��
	// 361 -- 	GPRS����ʱ��
	// 362 -- 	GPRS ���͵������ʱ��
	// 363 -- GPRS ���͵��������ʱ��
	
	
}BmsReg_deviceInfo;

//BMS���ƼĴ���
typedef struct
{
	/*
	512 ~ 556
	*/
	/*
		BMS������
		Bit0 : R/W 	 CHG_EN ��翪�ؿ��ƣ�1--��絼ͨ,0--���ر�
		Bit1 : R/W 	 DIS_EN �ŵ翪�ؿ��ƣ�1--�ŵ絼ͨ,0--�ŵ�ر�
		Bit2 : R/W	 PRE_CHG_EN :Ԥ�ŵ翪�ؿ��ƣ�1:��ͨ��0:�ر�
		Bit3,Bit4 : R/W	 ����
		Bit5 : R/W	1:ǳ����,0:��ع���
		Bit6 : R/W  1:�������,0:��ع���
		Bit7 : R/W  1:���������,0:�����δ����
		Bit8 ~ Bit14:����
		Bit15 : ʹ�ܾ������ģʽ
	*/
	uint16_t ctrl;
	uint16_t bmscer[8];  // BMS ��֤�֣�����BMS��֤Host , bmscer[7]:��д������Ϊ��д16����֤��д���
	uint16_t hostcer[8]; // Host ��֤�֣����ڣ��������֤����
	uint16_t afeccal; // У׼AFE�������� 0.01A/Bit ,W:д�뵱ǰʵ�ʵ���ֵ��ʼУ׼AFE��R��0--У׼���,others--����У׼
	uint16_t b16vcal; // У׼��16�ڵ�ص�ѹ����
	uint16_t yrmo;		// ��/�� BCD��
	uint16_t dthr;		// ��/ʱ BCD��
	uint16_t mnsc;		// ��/�� BCD��
	uint16_t reset;		// ��λ 1s��λ������ӦӦ��
	uint16_t record;	// ��¼����(����)
	uint16_t balctl;    // �������(����)
	uint16_t tctrl;		// ���Կ���
	uint16_t tres;		// ���Խ�� 
	// 539 ~ 554 		����
	// 555 	-- B4/B5 BMSʹ��
	// 556 --- ��17�ڵ�ؾ������(����)
}BmsReg_ctrl;

//BMS���üĴ���
typedef struct
{
	/*
		ADDR:768 ~ 4116
	*/
	/*
	ADDR:768 ����ʧ�洢������
	W:
	0 -- ��Ч
	0x1235 -- 
	*/
	uint16_t nvmcmd; 
	uint16_t rwsn[4];  // ���к�
	uint16_t sovp;		// �����ѹ����ֵ(1s) 1mv/Bit
	uint16_t sovpr;		// �����ѹ�����ָ�ֵ 
	uint16_t suvp1;		// ����Ƿѹ����
	uint16_t suvpt1;
	uint16_t suvpr1;
	uint16_t suvpsoc1;
	uint16_t suvpsoct1;
	uint16_t suvpsocr1;
	uint16_t suvp2;
	uint16_t suvpr2;
	uint16_t cocp;
	uint16_t cocpt;
	uint16_t docp1;
	uint16_t docpt1;
	uint16_t docp2;
	uint16_t dscp;
	uint16_t cotp;
	uint16_t cotpt;
	uint16_t cotpr;
	uint16_t dotp;
	uint16_t dotpt;
	uint16_t dotpr;
	uint16_t cutp;
	uint16_t cutpt;
	uint16_t cutpr;
	uint16_t dutp;
	uint16_t dutpt;
	uint16_t dutpr;
	uint16_t mosotp;
	uint16_t mosotpt;
	uint16_t mosotpr;
	uint16_t conotp;
	uint16_t conotpt;
	uint16_t conotpr;
	uint16_t sovw;
	uint16_t sovwt;
	uint16_t sovwr;
	uint16_t suvw;
	uint16_t suvwt;
	uint16_t suvwr;
	uint16_t cocw;
	uint16_t cocwt;
	uint16_t cocwr;
	uint16_t docw;
	uint16_t docwt;
	uint16_t docwr;
	uint16_t cotw;
	uint16_t cotwt;
	uint16_t cotwr;
	uint16_t dotw;
	uint16_t dotwt;
	uint16_t dotwr;
	uint16_t cutw;
	uint16_t cutwt;
	uint16_t cutwr;
	uint16_t dutw;
	uint16_t dutwt;
	uint16_t dutwr;
	uint16_t afecg;
	uint16_t b16vg;
	uint16_t cycles;
	uint16_t tvsotp;
	uint16_t tvsotpt;
	uint16_t tvsotpr;
}BmsReg_cfg;

//BMS�û���Ϣ���Ĵ���

typedef struct
{
	uint16_t udcmd;
	uint16_t userID[4];
}BmsReg_user;

#define BMS_REG_ID_ADDR 0

#define BMS_REG_INFO_ADDR_1 256
#define BMS_REG_INFO_ADDR_2 285
#define BMS_REG_INFO_END    316//322

#define BMS_REG_CTRL_ADDR   512
#define BMS_REG_CFG_ADDR    768
#define BMS_REG_USER_ADDR   4096

//�Ĵ�����ַ����
extern const BmsReg_info* g_bmsIICReg_id_addrMap;
extern const BmsReg_deviceInfo* g_bmsIICReg_Info_addrMap;
extern const BmsReg_ctrl* g_bmsIICReg_ctrl_addrMap;
extern const BmsReg_cfg* g_bmsIICReg_cfg_addrMap;
extern const BmsReg_user* g_bmsIICReg_info_addrMap;

#pragma pack()
#endif

/*****************************************end of MBFindMap.h*****************************************/
