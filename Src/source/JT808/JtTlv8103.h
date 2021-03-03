
#ifndef  _JT_TLV_8103_H_
#define  _JT_TLV_8103_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "utp.h"
#include "TlvIn.h"
#include "TlvOut.h"
	
	typedef struct _JtDevCfgParam
	{
		uint32 hbIntervalS   ;		
		uint32 tcpWaitRspTime;
		uint32 tcpReTxCount  ;
		char   mainSvrUrl[256]    ;
		uint32 mainSvrPort   ;
		uint32 locReportWay;
		uint32 locReportPlan;
		uint32 sleepLocReportInterval;
		uint32 urgLocReportInterval;
		uint32 defLocReportInterval;
		uint32 gpsSampFre;
		uint32 devType;
		uint32 factoryFlag;
		uint8  devmodule[8];
	}JtDevCfgParam;

	
	
	//TAG�Ǵ�˸�ʽ
	typedef enum
	{
		TAG_HB_INTERVAL	   = 0x0100,		//�ն��������ͼ������λΪ��(s)
		TAG_TCP_RSP_TIME   = 0x0200,		//TCP ��ϢӦ��ʱʱ�䣬��λΪ��(s)
		TAG_TCP_RETX_COUNT = 0x0300,		//TCP ��Ϣ�ش�����
		TAG_MAIN_SVR_URL   = 0x1300,		//����������ַ�� IP ������
		TAG_MAIN_SVR_PORT  = 0x1800,		//���������˿�
		TAG_LOC_REPORT_WAY		= 0x2000,	//λ�û㱨���ԣ� 0����ʱ�㱨�� 1������㱨�� 2����ʱ�Ͷ���㱨��
		TAG_LOC_REPORT_PLAN		= 0x2100,	//λ�û㱨������ 0����ʱ�㱨�� 1������㱨�� 2����ʱ�Ͷ���㱨��
		TAG_SLEEP_LOC_INTERVAL	= 0x2700,	//����ʱ�㱨ʱ��������λΪ��(s),>0��
		TAG_URG_LOC_INTERVAL	= 0x2800,	//����ʱ�㱨ʱ��������λΪ��(s),>0��
		TAG_DEF_LOC_INTERVAL	= 0x2900,	//Ĭ�ϻ㱨ʱ��������λΪ��(s),>0��
		TAG_GPS_SAMPL_FREQUENCY	= 0x2A00,	// GPS��������
		TAG_DEVICE_TYPE		 = 0x3000,		//�豸���͡�
		TAG_FACTORY_CFG_FLAG = 0x3100,		//�������ñ�־��0-�������ã�1-�ǳ������á�
		TAG_DEVICE_MODLE 	= 0x3200		// �ն��ͺ�
		//δ�����������
	}CMD8103_TLV;

	void JtTlv8103_init();
	void JtTlv8103_updateMirror(const uint8* data, int len);
	int JtTlv8103_getChanged(uint8* buf, int len, uint8* tlvCount);
	int JtTlv8103_getDefChanged(uint8* buf, int len);
	void JtTlv8103_updateStorage(const uint8* data, int len);
	uint32 JtTlv8103_getFactoryCofnig(void);

	extern TlvInMgr  g_jtTlvInMgr_8103;
	extern TlvOutMgr g_jtTlvOutMgr_8103;

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


