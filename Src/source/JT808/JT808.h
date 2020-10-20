
#ifndef  _JT808_H_
#define  _JT808_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "HwFwVer.h"
#include "JtTlv8103.h"

	typedef enum
	{
		JTCMD_SIM_HB		= 0x01,
		JTCMD_MCU_HB		= 0x02,
		JTCMD_SET_OP_STATE	= 0x03,

		JTCMD_CMD_GET_SIM_ID  = 0x11,
		JTCMD_CMD_GET_SIM_CFG,
		JTCMD_CMD_SET_SIM_CFG,
		JTCMD_CMD_GET_SMS,
		JTCMD_CMD_SEND_TO_SVR = 0x15,

		JTCMD_CMD_GET_FILE_INFO = 0X16,

		JTCMD_CMD_GET_FILE_CONTENT = 0X17,
		JTCMD_CMD_SET_LOCATION_EXTRAS = 0x18,

		JTCMD_CMD_GET_BLE_ID = 0x30,
		JTCMD_CMD_GET_BLE_CFG,
		JTCMD_CMD_SET_BLE_CFG,
		JTCMD_CMD_GET_BLE_EN,
		JTCMD_CMD_SET_BLE_EN,
		
		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,
		JTCMD_EVENT_DEV_STATE_LOCATION,

		JTCMD_BLE_EVT_AUTH = 0x90,
		JTCMD_BLE_EVT_CNT = 0x91,
		JTCMD_BLE_EVT_BEACON = 0x92,
		JTCMD_BLE_RCV_DAT = 0x93,

		JTCMD_EVT_RCV_SVR_DATA =  0xA0,
		JTCMD_EVT_RCV_FILE_DATA ,
	}JT_cmd;

	typedef enum
	{
		JT_STATE_INIT			= 0x00,
		JT_STATE_SLEEP			= 0x01,
		JT_STATE_WAKEUP			= 0x02,
		JT_STATE_PREOPERATION	= 0x04,
		JT_STATE_OPERATION		= 0x05,

		JT_STATE_UNKNOWN		= 0xFF,
	}JT_state;

#define			_BIT(x,n)			((x) << (n))
#define			_SET_BIT(n)			_BIT(1,n)
		
	
	// 0 ~ 1 GPRS״̬
#define			_GPRS_POWEROFF		_BIT(0,0)	// GPRS ��Դ�ر�
#define			_GPRS_LOWMODE		_BIT(1,0)	// GPRS �͹���,TCP�����ж�,���ſɻ���
#define			_GPRS_RESERVED		_BIT(2,0)	// ����
#define			_GPRS_NORMAL		_BIT(3,0)	// ȫ����
	// 2 ~ 3 GPS״̬
#define			_GPS_POWEROFF		_BIT(0,2)	// GPS �ر�
#define			_GPS_LOWMODE		_BIT(1,2)	// GPS �͹���
#define			_GPS_RESERVED		_BIT(2,2)	// ����
#define			_GPS_NORMAL			_BIT(3,2)	// ȫ����
	// 4 ~ 5 BLE ״̬
#define			_BLE_POWEROFF		_BIT(0,4)	// �ر�BLE��Դ
#define			_BLE_LOWMODE		_BIT(1,4)	// BLE�͹���,�㲥,�ȴ�����
#define			_BLE_RESERVED		_BIT(2,4)	// ����
#define			_BLE_NORMAL			_BIT(3,4)	// ȫ����
		
		
	
	
#define			_NETWORK_CONNECTION_BIT			_SET_BIT(0)
#define			_GPS_FIXE_BIT					_SET_BIT(1)
#define			_SMS_EXIT_BIT					_SET_BIT(2)


#pragma pack(1)

	typedef struct
		{
			uint8_t 	OperationState;
			uint8_t 	StateParameter;
		}JT_SetOperationState;

		typedef struct _JT_devState
		{
			uint16_t cnt;	// �豸״̬-GPRS,GPS,SMS
			uint8_t csq;	// GPRS/4G �ź�ǿ��
			uint8_t snr;	// GPS �����
			uint8_t siv;	// GPS �ɼ���
		}JT_devState;
		
	//�ն˵��豸���Զ���
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint16  protocolVer;	//��?��?D-������?��?o?
		uint32 devClass;	//????����D��
		uint8  vendorID[5];	//???������ID
		uint8  devModel[20];//????D��o?
		char  devId[12];	//???? ID
		char  iccid[10];	//???? SIM ?�� ICCID
		char  hwVer[JT_DEV_HW_VER_SIZE];	//????��2?t��?��?o?,��?0?��?2
		char  fwVer[JT_DEV_FW_VER_SIZE];	//????1��?t��?��?o?,��?0?��?2
	}JtDevProperty;
	// Jose add 2020/09/17
	// ????��??��
#define		JT_DEV_BLE_MAC_ADDR_SIZE	6
#define		JT_DEV_BLE_HW_VER_SIZE		16
#define		JT_DEV_BLE_FW_VER_SIZE		16

	typedef struct _JtDevBleProperty
	{
		uint16 BleType ;
		char  BleMac[JT_DEV_BLE_MAC_ADDR_SIZE];
		char  BlehwVer[JT_DEV_BLE_HW_VER_SIZE];
		char  BlefwVer[JT_DEV_BLE_FW_VER_SIZE];
	}JtDevBleProperty;
	
	// Jose add 2020/09/17
	typedef struct _JtDevBleCfgParam
	{
		char BleName[32];		// �����㲥����
		uint32 BleAdvInterval ;	// �㲥���(1ms)
		uint8 BleAdvPower ;		// �ļ����书��
		uint8 BleAdvData[31];	// �㲥����
	}JtDevBleCfgParam;

	typedef struct 
	{
		uint32 Updatefilelength ;
		uint8  UpdatefileVersion;
	}UpdateFileInfo;

	typedef struct
	{
		uint8	fileType;
		uint32 	fileOffset;
	}FileContent;

	// ?��??��?����
	typedef struct
	{
		int32 longitude ;	// ?-?���̣�???a?��,?a����?��?�� x 1E7 , 0 ������????����?
		int32 latitude ;	// ?3?��
	}Jt_LocationData;

#define		_BLE_CONNECT_BIT		BIT(0)
	// ble���䨬?
	typedef struct
	{
		uint8 bleConnectState ;
		uint8 bleConnectMAC[6];
	}Jt_BleState;

// ???��������?���䨬??����?
#define			_STATE_ACC				BIT(0)		// ACC 
#define			_STATE_WHEL_LLOCK		BIT(1)		// ??����
#define			_STATE_CAB_LOCK			BIT(2)		//	����2???���䨬?
#define			_STATE_REPOWER_OFF		BIT(3)		//  ??3��??��?
#define			_STATE_ACTIVE			BIT(4)		//	?��??���䨬?
#define			_STATE_BAT_AUTH_EN		BIT(5)		//	?��??���䨬?
#define			_STATE_ALAM_EN			BIT(6)		//	��??��


#define		_SMS_SIZE		(140)
	typedef struct
	{
		uint32 	smsRevTime;
		uint8	smsLen;
		uint8	smsText[_SMS_SIZE];
	}SMSContext;
	#define		_SMS_ARRY_CNT	(2)
	typedef struct
	{
		uint8 		smsExist;
		uint8 		smsCount;
		SMSContext	smsText[_SMS_ARRY_CNT];
	}GetSMSContext;

// ???������?�����䨬??����?



	// Jose add end
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//��ǰJT808�Ĳ���״̬
		JT_state opState;
		//����JT808�Ĳ���״̬
		//JT_state setToOpState;
		JT_SetOperationState setToOpState;

		JT_devState devState;

		Jt_LocationData locatData ;
		
		//�Ƿ�λ�ɹ�
		//Bool isLocation;
		
		//�ն���������
		JtDevProperty property;
		// ble����
		Jt_BleState		bleState ;
		uint16_t 		bleEnCtrl;
		JtDevBleProperty bleproperty ;
		JtDevBleCfgParam blecfgParam ;
		
		GetSMSContext	smsContext ;
		
		// �ļ�����
		UpdateFileInfo  updatefileinfo ;
		FileContent		filecontent ;
		//

		JT808fsmFn fsm;

	}JT808;

	extern JT808* g_pJt;
	extern JT808 g_Jt;
	void JT808_init();

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


