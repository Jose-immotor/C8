
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
		
	
	// 0 ~ 1 GPRS状态
#define			_GPRS_POWEROFF		_BIT(0,0)	// GPRS 电源关闭
#define			_GPRS_LOWMODE		_BIT(1,0)	// GPRS 低功耗,TCP连接中断,短信可唤醒
#define			_GPRS_RESERVED		_BIT(2,0)	// 保留
#define			_GPRS_NORMAL		_BIT(3,0)	// 全功能
	// 2 ~ 3 GPS状态
#define			_GPS_POWEROFF		_BIT(0,2)	// GPS 关闭
#define			_GPS_LOWMODE		_BIT(1,2)	// GPS 低功耗
#define			_GPS_RESERVED		_BIT(2,2)	// 保留
#define			_GPS_NORMAL			_BIT(3,2)	// 全功能
	// 4 ~ 5 BLE 状态
#define			_BLE_POWEROFF		_BIT(0,4)	// 关闭BLE电源
#define			_BLE_LOWMODE		_BIT(1,4)	// BLE低功耗,广播,等待连接
#define			_BLE_RESERVED		_BIT(2,4)	// 保留
#define			_BLE_NORMAL			_BIT(3,4)	// 全功能
		
		
	
	
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
			uint16_t cnt;	// 设备状态-GPRS,GPS,SMS
			uint8_t csq;	// GPRS/4G 信号强度
			uint8_t snr;	// GPS 信噪比
			uint8_t siv;	// GPS 可见星
		}JT_devState;
		
	//终端的设备属性定义
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint16  protocolVer;	//传输协议版本号
		uint32 devClass;	//终端类型
		uint8  vendorID[5];	//制造商ID
		uint8  devModel[20];//终端型号
		uint8  devId[12];	//终端 ID
		uint8  iccid[10];	//终端 SIM 卡 ICCID
		uint8  hwVer[JT_DEV_HW_VER_SIZE];	//终端硬件版本号,以0结尾
		uint8  fwVer[JT_DEV_FW_VER_SIZE];	//终端固件版本号,以0结尾
	}JtDevProperty;
	// Jose add 2020/09/17
	// 终端蓝牙
#define		JT_DEV_BLE_MAC_ADDR_SIZE	6
#define		JT_DEV_BLE_HW_VER_SIZE		16
#define		JT_DEV_BLE_FW_VER_SIZE		16

	typedef struct _JtDevBleProperty
	{
		uint16 BleType ;
		uint8  BleMac[JT_DEV_BLE_MAC_ADDR_SIZE];
		uint8  BlehwVer[JT_DEV_BLE_HW_VER_SIZE];
		uint8  BlefwVer[JT_DEV_BLE_FW_VER_SIZE];
	}JtDevBleProperty;
	
	// Jose add 2020/09/17
	typedef struct _JtDevBleCfgParam
	{
		uint8 BleName[32];		// 蓝牙广播名称
		uint32 BleAdvInterval ;	// 广播间隔(1ms)
		uint8 BleAdvPower ;		// 文件发射功率
		uint8 BleAdvData[31];	// 广播内容
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

	// 定位坐标
	typedef struct
	{
		int32 longitude ;	// 经度单位为度,为实际值 x 1E7 , 0 表示没定义
		int32 latitude ;	// 纬度
	}Jt_LocationData;

#define		_BLE_CONNECT_BIT		BIT(0)
	// ble状态
	typedef struct
	{
		uint8 bleConnectState ;
		uint8 bleConnectMAC[6];
	}Jt_BleState;

// 附加设备状态定义
#define			_STATE_ACC				BIT(0)		// ACC 
#define			_STATE_WHEL_LLOCK		BIT(1)		// 轮毂
#define			_STATE_CAB_LOCK			BIT(2)		//	座舱锁状态
#define			_STATE_REPOWER_OFF		BIT(3)		//  远程断电
#define			_STATE_ACTIVE			BIT(4)		//	激活状态
#define			_STATE_BAT_AUTH_EN		BIT(5)		//	激活状态
#define			_STATE_ALAM_EN			BIT(6)		//	移动


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

// 附加报警状态定义



	// Jose add end
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//当前JT808的操作状态
		JT_state opState;
		//设置JT808的操作状态
		//JT_state setToOpState;
		JT_SetOperationState setToOpState;

		JT_devState devState;

		Jt_LocationData locatData ;
		
		//是否定位成功
		//Bool isLocation;
		
		//终端属性数据
		JtDevProperty property;
		// ble属性
		Jt_BleState		bleState ;
		uint16_t 		bleEnCtrl;
		JtDevBleProperty bleproperty ;
		JtDevBleCfgParam blecfgParam ;
		
		GetSMSContext	smsContext ;
		
		// 文件属性
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


