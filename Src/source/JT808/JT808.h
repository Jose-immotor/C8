
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
		JT_STATE_INIT			= 0x01,
		JT_STATE_SLEEP			,//= 0x02,
		JT_STATE_WAKEUP			,//= 0x03,
		JT_STATE_PREOPERATION	,//= 0x04,
		JT_STATE_OPERATION		,//= 0x05,

		JT_STATE_UNKNOWN		= 0xFF,
	}JT_state;



#define			_NETWORK_CONNECTION_BIT			BIT(0)
#define			_GPS_FIXE_BIT					BIT(1)
#define			_SMS_EXIT_BIT					BIT(2)

	typedef struct _JT_devState
	{
		uint16_t cnt;	// ¨¢??¨®¡Á¡ä¨¬?
		uint8_t csq;	// GPRS/4GD?o????¨¨
		uint8_t snr;	// GPSD?o????¨¨
		uint8_t siv;	// GPS?¨¦???¨¤D?
	}JT_devState;

#pragma pack(1)


	//????¦Ì?¨¦¨¨¡À?¨º?D??¡§¨°?
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint16  protocolVer;	//¡ä?¨º?D-¨°¨¦¡ã?¡À?o?
		uint32 devClass;	//????¨¤¨¤D¨ª
		uint8  vendorID[5];	//???¨¬¨¦¨¬ID
		uint8  devModel[20];//????D¨ªo?
		char  devId[12];	//???? ID
		char  iccid[10];	//???? SIM ?¡§ ICCID
		char  hwVer[JT_DEV_HW_VER_SIZE];	//????¨®2?t¡ã?¡À?o?,¨°?0?¨¢?2
		char  fwVer[JT_DEV_FW_VER_SIZE];	//????1¨¬?t¡ã?¡À?o?,¨°?0?¨¢?2
	}JtDevProperty;
	// Jose add 2020/09/17
	// ????¨¤??¨¤
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
		char BleName[32];		// ¨¤??¨¤1?2£¤??3?
		uint32 BleAdvInterval ;	// 1?2£¤????(1ms)
		uint8 BleAdvPower ;		// ???t¡¤¡é¨¦?1|?¨º
		uint8 BleAdvData[31];	// 1?2£¤?¨²¨¨Y
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

	// ?¡§??¡Á?¡À¨º
	typedef struct
	{
		int32 longitude ;	// ?-?¨¨¦Ì£¤???a?¨¨,?a¨º¦Ì?¨º?¦Ì x 1E7 , 0 ¡À¨ª¨º????¡§¨°?
		int32 latitude ;	// ?3?¨¨
	}Jt_LocationData;

#define		_BLE_CONNECT_BIT		BIT(0)
	// ble¡Á¡ä¨¬?
	typedef struct
	{
		uint8 bleConnectState ;
		uint8 bleConnectMAC[6];
	}Jt_BleState;

// ???¨®¨¦¨¨¡À?¡Á¡ä¨¬??¡§¨°?
#define			_STATE_ACC				BIT(0)		// ACC 
#define			_STATE_WHEL_LLOCK		BIT(1)		// ??¨¬¡À
#define			_STATE_CAB_LOCK			BIT(2)		//	¡Á¨´2???¡Á¡ä¨¬?
#define			_STATE_REPOWER_OFF		BIT(3)		//  ??3¨¬??¦Ì?
#define			_STATE_ACTIVE			BIT(4)		//	?¡è??¡Á¡ä¨¬?
#define			_STATE_BAT_AUTH_EN		BIT(5)		//	?¡è??¡Á¡ä¨¬?
#define			_STATE_ALAM_EN			BIT(6)		//	¨°??¡¥


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

// ???¨®¡À¡§?¡¥¡Á¡ä¨¬??¡§¨°?



	// Jose add end
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//¦Ì¡À?¡ãJT808¦Ì?2¨´¡Á¡Â¡Á¡ä¨¬?
		JT_state opState;
		//¨¦¨¨??JT808¦Ì?2¨´¡Á¡Â¡Á¡ä¨¬?
		JT_state setToOpState;

		JT_devState devState;

		Jt_LocationData locatData ;
		
		//¨º?¡¤??¡§??3¨¦1|
		//Bool isLocation;
		
		//????¨º?D?¨ºy?Y
		JtDevProperty property;
		// ble¨º?D?
		Jt_BleState		bleState ;
		uint16_t 		bleEnCtrl;
		JtDevBleProperty bleproperty ;
		JtDevBleCfgParam blecfgParam ;
		
		GetSMSContext	smsContext ;
		
		// ???t¨º?D?
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


