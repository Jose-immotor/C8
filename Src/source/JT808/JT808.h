
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
		
	
	// 0 ~ 1 GPRS×´Ì¬
#define			_GPRS_POWEROFF		_BIT(0,0)	// GPRS ï¿½ï¿½Ô´ï¿½Ø±ï¿½
#define			_GPRS_LOWMODE		_BIT(1,0)	// GPRS ï¿½Í¹ï¿½ï¿½ï¿½,TCPï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½,ï¿½ï¿½ï¿½Å¿É»ï¿½ï¿½ï¿½
#define			_GPRS_RESERVED		_BIT(2,0)	// ï¿½ï¿½ï¿½ï¿½
#define			_GPRS_NORMAL		_BIT(3,0)	// È«ï¿½ï¿½ï¿½ï¿½
	// 2 ~ 3 GPS×´Ì¬
#define			_GPS_POWEROFF		_BIT(0,2)	// GPS ï¿½Ø±ï¿½
#define			_GPS_LOWMODE		_BIT(1,2)	// GPS ï¿½Í¹ï¿½ï¿½ï¿½
#define			_GPS_RESERVED		_BIT(2,2)	// ï¿½ï¿½ï¿½ï¿½
#define			_GPS_NORMAL			_BIT(3,2)	// È«ï¿½ï¿½ï¿½ï¿½
	// 4 ~ 5 BLE ×´Ì¬
#define			_BLE_POWEROFF		_BIT(0,4)	// ï¿½Ø±ï¿½BLEï¿½ï¿½Ô´
#define			_BLE_LOWMODE		_BIT(1,4)	// BLEï¿½Í¹ï¿½ï¿½ï¿½,ï¿½ã²¥,ï¿½È´ï¿½ï¿½ï¿½ï¿½ï¿½
#define			_BLE_RESERVED		_BIT(2,4)	// ï¿½ï¿½ï¿½ï¿½
#define			_BLE_NORMAL			_BIT(3,4)	// È«ï¿½ï¿½ï¿½ï¿½
		
		
#define			_OPERATION_INIT		_BLE_POWEROFF | _GPS_POWEROFF | _GPRS_POWEROFF	
#define			_OPERATION_PRE		_BLE_POWEROFF | _GPS_POWEROFF | _GPRS_POWEROFF
#define			_OPERATION_OPE		_BLE_NORMAL | _GPS_NORMAL | _GPRS_NORMAL
#define			_OPERATION_WKUP		_BLE_LOWMODE | _GPS_POWEROFF | _GPRS_LOWMODE
#define			_OPERATION_SLEEP	_BLE_LOWMODE | _GPS_POWEROFF | _GPRS_LOWMODE



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
			uint16_t cnt;	// ï¿½è±¸×´Ì¬-GPRS,GPS,SMS
			uint8_t csq;	// GPRS/4G ï¿½Åºï¿½Ç¿ï¿½ï¿½
			uint8_t snr;	// GPS ï¿½ï¿½ï¿½ï¿½ï¿?
			uint8_t siv;	// GPS ï¿½É¼ï¿½ï¿½ï¿½
		}JT_devState;
		
	//ï¿½Õ¶Ëµï¿½ï¿½è±¸ï¿½ï¿½ï¿½Ô¶ï¿½ï¿½ï¿½
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint16_t  protocolVer;	//Ð­Òé°æ±¾ºÅ
		uint32_t devClass;	//ÖÕ¶ËÀàÐÍ
		uint8_t  vendorID[5];	//ÖÆÔìÉÌID
		uint8_t  devModel[8];// [8] ÖÕ¶ËÐÍºÅ
		uint8_t  devId[12];	//ÖÕ¶ËID
		uint8_t  iccid[10];	//ICCID
		uint8_t  hwVer[JT_DEV_HW_VER_SIZE];	//Ó²¼þ°æ±¾ºÅ
		uint8_t  fwVer[JT_DEV_FW_VER_SIZE];	//¹Ì¼þ°æ±¾ºÅ
	}JtDevProperty;
	// Jose add 2020/09/17
	// ????ï¿½ï¿½??ï¿½ï¿½
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
		char BleName[32];		// ï¿½ï¿½ï¿½ï¿½ï¿½ã²¥ï¿½ï¿½ï¿½ï¿½
		uint32 BleAdvInterval ;	// ï¿½ã²¥ï¿½ï¿½ï¿?1ms)
		uint8 BleAdvPower ;		// ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ä¹¦ï¿½ï¿½
		uint8 BleAdvData[62];	// ï¿½ã²¥ï¿½ï¿½ï¿½ï¿½
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
	}FileContentReq;

	typedef struct
	{
		uint32 fileOffset ;
		uint8  fileData[128];
	}FileContentRsq;

	typedef struct
	{
		uint8 fileType ;
		uint8 fileVerDesc ;
	}FileVersionDesc;

	

	// ?ï¿½ï¿½??ï¿½ï¿½?ï¿½ï¿½ï¿½ï¿½
	typedef struct
	{
		int32 longitude ;	// ?-?ï¿½ï¿½ï¿½Ì£ï¿½???a?ï¿½ï¿½,?aï¿½ï¿½ï¿½ï¿½?ï¿½ï¿½?ï¿½ï¿½ x 1E7 , 0 ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½ï¿½ï¿½?
		int32 latitude ;	// ?3?ï¿½ï¿½
	}Jt_LocationData;

#define		_BLE_CONNECT_BIT		BIT(0)
	// bleï¿½ï¿½ï¿½ä¨¬?
	typedef struct
	{
		uint8 bleConnectState ;
		uint8 bleConnectMAC[6];
	}Jt_BleState;

// ???ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?ï¿½ï¿½ï¿½ä¨¬??ï¿½ï¿½ï¿½ï¿½?
#define			_STATE_ACC				BIT(0)		// ACC 
#define			_STATE_WHEL_LLOCK		BIT(1)		// ??ï¿½ï¿½ï¿½ï¿½
#define			_STATE_CAB_LOCK			BIT(2)		//	ï¿½ï¿½ï¿½ï¿½2???ï¿½ï¿½ï¿½ä¨¬?
#define			_STATE_REPOWER_OFF		BIT(3)		//  ??3ï¿½ï¿½??ï¿½ï¿½?
#define			_STATE_ACTIVE			BIT(4)		//	?ï¿½ï¿½??ï¿½ï¿½ï¿½ä¨¬?
#define			_STATE_BAT_AUTH_EN		BIT(5)		//	?ï¿½ï¿½??ï¿½ï¿½ï¿½ä¨¬?
#define			_STATE_ALAM_EN			BIT(6)		//	ï¿½ï¿½??ï¿½ï¿½


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
		SMSContext	smsText;//[_SMS_ARRY_CNT];
	}GetSMSContext;

// ???ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?ï¿½ï¿½ï¿½ï¿½ï¿½ä¨¬??ï¿½ï¿½ï¿½ï¿½?
//
#define		_BLE_CTL_SCAN			BIT(0)	// É¨ÃèÐÅ±ê
#define		_BLE_CTL_RESET			BIT(1)	// ÖØÆô
#define		_BLE_CTL_FILTER			BIT(2)	// ÐÅ±ê¹ýÂÇ 



// Beacon
	typedef struct
	{
		int8		miRSSI;		// ÐÅºÅÇ¿¶È
		uint8		mADV[62];	// ¹ã²¥Êý¾Ý
	}BLEAdvContext;	

	#define		_BLE_BEACON_CNT			(4)
	typedef struct
	{
		//uint8 			BeaconCntextLen;
		BLEAdvContext	BeaconADV[_BLE_BEACON_CNT];
	}BeaconCntext ;

	typedef struct
	{
		uint8 	extraStType;	// 0xF0
		uint8	extraStLen;	// 0x04
		uint32 	extraStStatus;
		uint8	extraAlType; // 0xF1
		uint8 	extraAlLen;	// 0x04
		uint32	extraAlam;
	}LocationExtras;
	
	// beacon
	typedef struct
	{
		uint8_t 	miUUID[16];
		uint16_t 	miMajor;
		uint16_t 	miMinor;
		int8_t		miRSSI;
		uint8_t		miSOC;
		uint16_t 	miVoltage;
	}BeaconCell;
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//ï¿½ï¿½Ç°JT808ï¿½Ä²ï¿½ï¿½ï¿½×´Ì¬
		JT_state opState;
		//ï¿½ï¿½ï¿½ï¿½JT808ï¿½Ä²ï¿½ï¿½ï¿½×´Ì¬
		//JT_state setToOpState;
		JT_SetOperationState setToOpState;

		JT_devState devState;

		Jt_LocationData locatData ;
		
		//ï¿½Ç·ï¿½Î»ï¿½É¹ï¿½
		//Bool isLocation;
		
		//ï¿½Õ¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		JtDevProperty property;
		// bleï¿½ï¿½ï¿½ï¿½
		Jt_BleState		bleState ;
		uint16_t 		bleEnCtrl;
		JtDevBleProperty bleproperty ;
		JtDevBleCfgParam blecfgParam ;
		BeaconCntext	 bleBeaconCntext;
		GetSMSContext	smsContext ;
		LocationExtras	locationExtras;
		
		// ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½
		UpdateFileInfo  updatefileinfo ;
		FileContentReq		filecontentreq ;
		FileContentRsq		filecontentrsq ;
		FileVersionDesc fileDesc;
		//

		JT808fsmFn fsm;

	}JT808;

	extern JT808* g_pJt;
	extern JT808 g_Jt;
	void JT808_init();
	void Jt808TaskInit(void);





#define		_BLE_TYPE			_SET_BIT(0)	// À¶ÑÀ
#define		_PMS_TYPE			_SET_BIT(1)	// PMS
#define		_ACC_TYPE			_SET_BIT(2)	// ÎïÀíµã»ð¼ü
#define		_LOCL_TYPE			_SET_BIT(3)	// ×ù²ÕËø
#define		_CABIN_TYPE			_SET_BIT(4)	// ÂÖì±Ëø

// ÖÕ¶ËÀàÐÍ
#define		_DEV_TYPE			_BLE_TYPE | _PMS_TYPE | _LOCL_TYPE
// ÖÕ¶ËÐÍºÅ
#define		_DEV_MODEL		"C8"


//BLE name
#define		_BLE_NAME		"IMT-%s-%02X%02X%02X"
#define		_BLE_COMPY		"30"



void SetLocationExtras(uint8 extras );

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


