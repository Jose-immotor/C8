
#ifndef __BLE__H_
#define __BLE__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "Pms.h"
#include "BleUser.h"

#define BLE_MAC_SIZE		6
#define SERIAL_NUM_SIZE		6

#define REQ_ID_AUTHR			0x01
#define REQ_ID_CLEAN_PAIRED		0x02
#define REQ_ID_GET_PORT_STATE	0x03
#define REQ_ID_GET_BATTERYINFO	0x04
#define REQ_ID_MODE_SWITCH		0x06
#define REQ_ID_GET_TESTRESULT	0x07
#define REQ_ID_GET_GPS_GPRS_INFO 0x08
#define REQ_ID_SETTING			0x09
#define REQ_ID_SET_BLE_PSW		0x0A
#define REQ_ID_GET_BLE_PSW		0x0B
#define REQ_ID_EVENT_IND		0x0C

#define REQ_ID_GET_DEVICECAP		0x18
#define REQ_ID_GET_DEVICEID			0x19
#define REQ_ID_GET_PMS_INFO			0x1A
#define REQ_ID_GET_BMS_INFO			0x1B
#define REQ_ID_ACTIVE_REQ			0x1C
#define REQ_ID_SIGNIN_REQ			0x1D
#define REQ_ID_BAT_VERIFY			0x1E
#define REQ_ID_UPDATE_APPROM_START	0x20
#define REQ_ID_UPDATE_APPROM		0x21
#define REQ_ID_UPDATE_APPROM_DONE	0x22
#define REQ_ID_RESET				0x23

#define REQ_ID_FILE_TRANS_START	0x24
#define REQ_ID_FILE_TRANS		0x25
#define REQ_ID_FILE_TRANS_DONE	0x26
#define REQ_ID_GET_FILEID		0x27
#define REQ_ID_GET_BIRGHTNESS	0x28
#define REQ_ID_SET_NVDS			0x29
#define REQ_ID_GET_NVDS			0x2A
#define REQ_ID_SET_FACTORY_SETTINGS			0x2B

#define REQ_ID_SET_ALARM_MODE	0x2C
#define REQ_ID_GET_RUNPARAM		0x36	
#define	REQ_ID_SET_WHEELLOCK	0x37
#define REQ_ID_SET_CABINLOCK	0x38
#define REQ_ID_SET_ACC			0x39

#define	REQ_ID_GET_BEACON_SCAN	0x3E	//
#define	REQ_ID_GET_BEACON_CMD	0x3F


#define REQ_ID_SET_IP				0xf1
#define REQ_ID_GET_IP				0xf2

/*Notify*/
#define NOTIFY_ID_HEARTBEAT		0xA0
#define NOTIFY_ID_PROMPTS		0xA1
#define NOTIFY_ID_CAPTURE		0xA2

/*RSP ERROR Code*/
#define SUCCESS					0x00
#define ERR_DEFAULT				0xff
#define ERR_PARA_INVALID		0x01
#define ERR_UNSUPPORTED			0x02
#define ERR_CHECKSUM			0x03
//#define ERR_OTHER_BINDED		0x04
#define ERR_USERID_LEN_INVALID  0x05
#define ERR_USERID_INVALID		0x0B
#define ERR_BAT_NOT_IN_PRESENT	0x0C
#define ERR_RECORD_INVALID		0x0D	//记录号无效
#define ERR_CMD_NOT_ALLOW		0x0E	//命令不允许执行

#pragma pack(1)

	//电池属性信息（不会在运行中改变）
	typedef struct _BatProperty
	{
		uint8 portNum;
		uint16 nominalVol;
		uint16 nominalCur;
		uint16 nominalCap;
	}BatProperty;

	typedef struct _PmsPortStatePkt
	{
		uint8 portState;
		uint8 portCount;
		BatProperty property[2];
	}PmsPortStatePkt;

	typedef struct _BleGetDevIDPkt
	{
		uint8 protocolVer;

		uint8 hwMainVer;
		uint8 hwSubVer;

		uint8 fwMainVer;
		uint8 fwSubVer;
		uint8 fwMinorVer;
		uint32 buildNum;
	}BleGetDevIDPkt;

	// 0x18 设备能力
	typedef struct
	{
		uint8 protocolVer ;
		uint32 capacity ;	// 能力描述
	}DevCapacity ;

	// 0x1b
	

	//Pms V39及以后的版本的电池描述符长度
	typedef struct _BatteryDesc
	{
		uint8  serialNum[SERIAL_NUM_SIZE];
		uint8  portId;
		uint8  SOC;
		uint16 voltage;	// 电压
		int16  current;	// 电流
		int8   temp;	// 电池温度
		uint8  fault;	// 电池故障
		uint8  damage;	// 电池损坏
		uint16 cycleCount;	// 循环次数
//		uint16 rateVoltage;
//		uint16 rateCurrent;
//		uint16 capacity;
//		uint16 maxVoltage;
//		uint16 minVoltage;
//		uint8  maxVolCell;
//		uint8  minVolCell;
//		int8   bmsPcbTemp;
//		int8   connectorTemp;
//		uint8  mosState;
	}BatteryDesc;

	typedef struct _otvItem
	{
		uint8_t item ;
		uint8_t len;
		uint8_t param ;
	}otvItem ;
			
	typedef struct _GpsPkt
	{
		uint8  CSQ;
		uint8  Satellites;
		uint8  SNR;
		int32  longitude;
		int32  latitude;
		int32  speed;
	}GpsPkt;

	typedef struct _PmsPkt
	{
		uint8  Version;
		uint8  HwMainVer;
		uint8  HwSubVer;
		uint8  AppMainVer;
		uint8  AppSubVer;
		uint8  AppMinorVer;
		uint8  AppBuildNum;
		uint8  State;
	}PmsPkt;

	typedef struct _SelfTestResult
	{
		uint8  simState;
		uint8  gprsState;
		uint8  gpsState;
		uint8  devState;
		uint8  _18650Vol;
		uint8  devState2;
		uint8  batVerify;
		uint8  periheral;
	}SelfTestResult;
	//
	typedef struct
	{
		uint8_t	cnt;
		struct BeaconDecs
		{
			uint16_t Major;
			uint16_t Minor;
			int8_t	 RSSI;
		}BeaconDecs[5];
	}BleBeaconPkt;


	typedef struct
	{
		uint8_t mac[6];
		uint8	soc;
		uint16_t vol;
		int8_t rssi;
	}BleBeaconPkt1;

	typedef struct
	{
		uint16_t major;
		uint16_t minor;
	}GetBeaconREQ;
	
#pragma pack()

	typedef struct _Ble
	{
		uint8 isConnected ;
		uint8 isLowpower;

		uint8 mac[6];
		BleUser user;

		char hwVer[10];
		char fwVer[30];

		BleGetDevIDPkt devIdPkt;
		BatteryDesc batDesc[MAX_BAT_COUNT];
		GpsPkt gpsPkt;
		PmsPkt pmsPkt;
		PmsPortStatePkt portState;
		//BatteryDesc		batterDesc;
		BleBeaconPkt	bleBeaconPkt;
		//BleBeaconPkt1	bleBeaconPkt1;
		GetBeaconREQ	bleBeaconReq;
		DevCapacity		devCapacity ;
		BleGetDevIDPkt	bmsPkt[MAX_BAT_COUNT] ;
		uint8			mActiveDevice ;// 激活
		uint8			BatVerify[2];	// 电池身份验证
		uint8			wheelLock ;		// 轮毂锁
	}Ble;

	extern Ble g_Ble;
	uint8* Ble_ReqProc(const uint8_t* pReq, int frameLen, uint8* rspLen);
    void Ble_init(uint8* mac);
	void Ble_Logout(void);

#ifdef __cplusplus
}
#endif

#endif
