#ifndef __PMS__H_
#define __PMS__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "common.h"
//#include "datarom.h"
#include "bmsfault.h"

#include "nfc_cmd_process.h"

//用于存储需要判断变化的数据，以比较
typedef struct{
	//SOC
	st_modbus_reg_unit soc;
	//故障//运行故障字1
	st_modbus_reg_unit fault[6];
	//电池在线
	unsigned char online; // 0, 离线；1,在线
	//电池在线防抖延迟计数
	unsigned int online_delay_cnt;
}st_base_param;

typedef uint8_t BOOL;

typedef enum
{
    EV_PMS_ONLINE                      = 1<<0,  /*!< Startup finished. */
    EV_MASTER_FRAME_RECEIVED           = 1<<1,  /*!< Frame received. */
    EV_MASTER_EXECUTE                  = 1<<2,  /*!< Execute function. */
    EV_MASTER_FRAME_SENT               = 1<<3,  /*!< Frame sent. */
    EV_MASTER_ERROR_PROCESS            = 1<<4,  /*!< Frame error process. */
    EV_MASTER_PROCESS_SUCESS           = 1<<5,  /*!< Request process success. */
    EV_MASTER_ERROR_RESPOND_TIMEOUT    = 1<<6,  /*!< Request respond timeout. */
    EV_MASTER_ERROR_RECEIVE_DATA       = 1<<7,  /*!< Request receive data error. */
    EV_MASTER_ERROR_EXECUTE_FUNCTION   = 1<<8,  /*!< Request execute function error. */
} pmsEventType;

extern st_base_param gl_base_param[NFC_READER_COUNT_MAX];

#define PROTOCOL_UTP	2

#define WAKEUP_IO_DELAY  	500		//

#define PMS_SLEEP_AT_ONCE	0
#define PMS_SLEEP_TIME		4000

#define PMS_TIMEID_HEARBEAT  1
#define PMS_TIMEID_START	 2
#define PMS_TIMEID_SLEEP 	 3
#define PMS_TIMEID_ACCOFF	 4

#define PMS_HEARBEAT_TIME 5000
#define PMS_FWUPDATE_TIME 6000

#define PMS_PROTOCOL_VER   0	//协议版本号

typedef enum _PMS_EVENT
{
	 PMS_EVENT_DEVICE_RESET 		= 0x01
	,PMS_EVENT_BAT_STATE_CHANGED 	= 0x02
	,PMS_EVENT_PORT_STATE_CHANGED 	= 0x03
	,PMS_EVENT_SLEEP_REQ			= 0x04
	,PMS_SWITCH_PROTOCOL_REQ		= 0x05
	,PMS_EVENT_LOG					= 0x07
	,PMS_BMS_FAULT_CHANGED			= 0x08
	#ifdef CFG_WHEEL_LOCK
	,PMS_SPEED_CHANGED				= 0x09
	#endif
	#ifdef CFG_ASY
	,PMS_ASY_INFO					= 0x0A
	,PMS_ASY_STATE					= 0x0B
	,PMS_EVENT_CHANGED				= 0x0C
	#endif
	
	,PMS_EVENT_MAX			
}PMS_EVENT;

typedef enum _PMS_CMD
{
	 PMS_CMD_GET_DEVICEID 		= 0x21
	,PMS_CMD_GET_BATTERY_INFO 	= 0x22
	,PMS_CMD_GET_PORT_STATE 	= 0x23
	,PMS_CMD_SET_PMS_SLEEP		= 0x24
//	,PMS_CMD_SET_PMS_DISCHARGE	= 0x25
	,PMS_CMD_SET_PMS_CHARGEING	= 0x26

	,PMS_CMD_BYPASS			= 0x27
	,PMS_CMD_UPGRADE_START	= 0x28
	,PMS_CMD_UPGRADE		= 0x29
	,PMS_CMD_UPGRADE_DONE	= 0x2A

	,PMS_CMD_SYNC_STATE			= 0x2B
	,PMS_CMD_SET_BAT_DISCHARGE	= 0x30
	,PMS_CMD_SET_STOP_CONDITION	= 0x31
	#ifdef CFG_WHEEL_LOCK
	,PMS_CMD_SET_LOCK			= 0x32
	#endif	
	#ifdef CFG_REMOTE_ACC
	,PMS_CMD_SET_ACC			= 0x33
	#endif
	#ifdef CFG_ASY
	,PMS_CMD_SET_ASY			= 0x34
	,PMS_CMD_SET_DISCHARGE		= 0x35
	#endif
	,PMS_CMD_MAX			
}PMS_CMD;

#define PACK_STATE_SLEEP 		1
#define PACK_STATE_CHARGE 		2
#define PACK_STATE_DISCHARGE 	3

#pragma pack(1)
#define MAX_BMS_COUNT 2

typedef struct _GprsBatteryDesc
{
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint8  portId;
	uint8  SOC;
	uint8 voltage[2];
	uint8  current[2];
	int8   temp;
	uint8  fault;
	uint8  damage;
	uint8 cycleCount[2];
//	uint16 rateVoltage;
//	uint16 rateCurrent;
//	uint16 capacity;
	uint8  maxVoltage[2];
	uint8  minVoltage[2];
	uint8  maxVolCell;
	uint8  minVolCell;
	int8   bmsPcbTemp;
	int8   connectorTemp;
	uint8  mosState;
	uint8  version;
	uint8  hwver[2];
	uint8  blver[2];
	uint8  fwver[2];
	uint8  fwminorver;
	uint8  fwbuidnum[4];
	
}GprsBatteryDesc;

#ifdef CFG_ASY	//Motor vehicle speed controller，电动车调速控制器
typedef struct
{
	uint8  hwVer;
	uint8  sn[14];
}AsyInfoPkt;
extern AsyInfoPkt  g_AsyInfo;

typedef struct
{
	uint16  state[4];
	uint16  speed;
	uint16  ctrl;
	uint16  speedLimit;
	uint16  odoMeter;
}AsyStatePkt;
extern AsyStatePkt  g_AsyState;

typedef struct
{
	uint16  ctrl;

	uint16  speedLimit;
	uint16  odoMeter;
}AsyCtrlPkt;
extern AsyCtrlPkt g_LocalAsyCtrl;
void Pms_SetAsy(const AsyCtrlPkt* pPkt);
#endif

typedef struct _PrtMask
{
	uint8 	port0:1;
	uint8 	port1:1;
	uint8 	reserved:6;
}PortMask;

typedef struct _BatFault
{
	uint8 	OVP:1;	//BIT[0]: 过压
	uint8 	UVP:1;	//BIT[1]: 欠压
	uint8 	OCP:1;	//BIT[2]: 过流
	uint8 	OTP:1;	//BIT[3]: 过温
	uint8 	UTP:1;	//BIT[4]: 低温
	uint8 	Other:1;	//BIT[5]: 其他
	uint8 	Reserved:2;	//保留
}BatFault;

typedef struct _DevState
{
	uint8 	isAccOn:1;
	uint8 	isAllowCharge:1;
	uint8 	isTestBat:1;
	uint8 	isLock:2;
	uint8 	isStop:1;
	uint8 	reserved:2;
}DevState;

typedef struct _PortDescs
{
	uint8 	portCount;
	uint8 	portMask;
//	VerDesc BmsVerDesc[MAX_BMS_COUNT];
}PortDescs;


//Pms V39及以后的版本的电池描述符长度
typedef struct _BatteryDesc
{
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint8  portId;
	uint8  SOC;
	uint16 voltage;
	int16  current;
	int8   temp;
	uint8  fault;
	uint8  damage;
	uint16 cycleCount;
	uint16 rateVoltage;
	uint16 rateCurrent;
	uint16 capacity;
	uint16 maxVoltage;
	uint16 minVoltage;
	uint8  maxVolCell;
	uint8  minVolCell;
	int8   bmsPcbTemp;
	int8   connectorTemp;
	uint8  mosState;
}BatteryDesc;

typedef struct _BatStatePkt
{
	uint8  deviceState;
	#ifdef CFG_WHEEL_LOCK
	uint16  Speed;
	uint8  reserved;
	#else
	uint8  reserved[3];
	#endif
	uint16  MaxOutPutCurrent;
	uint16 workVoltate;
	int16  workCurrent;
	uint8  packState;
	uint8  batteryCount;
	uint8  size;
	BatteryDesc desc[NFC_READER_COUNT_MAX];
}BatStatePkt;

#define BAT_PKT_HEAD (sizeof(BatStatePkt) - sizeof(BatteryDesc)*2)
	
typedef struct _PortDesc
{
	uint8 portId;
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint16 rateVoltage;
	uint16 rateCurrent;
	uint16 capacity;
}PortDesc;

typedef struct _PortStateChangedPkt
{
	uint8  portMask;
	PortDesc portDesc[NFC_READER_COUNT_MAX];
}PortStateChangedPkt;
typedef struct _PortStateDesc
{
	uint8 portId;
	uint16 rateVoltage;
	uint16 rateCurrent;
	uint16 capacity;
}PortStateDesc;
typedef struct _PortState
{
	uint8  portState;
	int16  workCurrent;
	uint8  packState;
	uint8  batteryCount;
	PortStateDesc portDesc[NFC_READER_COUNT_MAX];
}PortState;

#define BAT_VERIFY_NONE 	0
#define BAT_VERIFY_OK		1
#define BAT_VERIFY_FAILED	2

#define LOCK_UNLOCK 	0
#define LOCK_LOCK   	1
#define LOCK_UNKNOWN   	2

#pragma anon_unions
#pragma pack()
typedef struct _Pms
{
	uint8 	m_isPowerOn:1;
	uint8 	m_isStart:1;
	uint8 	m_isNotDischarge:1;
	uint8 	m_PowerOffAtOnce:1;			//是否立即断电
	uint8 	m_Port0Verify:2;		//BAT_VERIFY_NONE - BAT_VERIFY_FAILED
	uint8 	m_Port1Verify:2;		//BAT_VERIFY_NONE - BAT_VERIFY_FAILED
	
	uint8 	m_isCommOk:2;
	uint8 	m_isGetDeviceReset:1;
	uint8 	m_isTestBat:1;
	uint8 	m_isSendStopCondition:1;
	uint8 	m_isLock:2;
	uint8 	m_reserved1:1;

#ifdef CFG_REMOTE_ACC
	uint8 	m_pmsAcc;
	uint8 	m_isBatSleep;
#endif

#ifdef CFG_NVC	
	uint8	m_playSoc;
#endif
	uint8	m_totalPort;
	uint8	m_portMask;
	DevState  m_deviceState;
	
	BatStatePkt m_batPkt;

	Bool m_isBatErr;
//	BmsFault m_BmsFault[MAX_BATTERY_COUNT];
}Pms;

extern Pms* g_pPms;

void Pms_Run(void);
void Pms_Start(void);
void Pms_Init(void);
void Pms_Sleep(uint32 ms);
void Pms_Wakeup(void);
void BatteryDump(const BatStatePkt* pPkt);
int Pms_GetBatInfo(uint8 batInd, void* pData);
int Pms_GetBatInfo1(uint8 batInd, void* pData);
int Pms_GetBatInfo2(uint8 batInd, void* pData);
int Pms_GetBatInfo3(uint8 batInd, void* pData);
int Pms_GetPortInfo(PortStateChangedPkt* portPkt);
void Pms_UtpRx(uint8 u8InChar);
void Pms_SwitchProtocol(uint8 protocol);
Bool Pms_IsAccOn(void);
uint8 Pms_GetBatCount(void);
Bool Pms_IsFwUpgrade(void);
void Pms_SetFwUpgrade(Bool isStart);
void Pms_UpgradePms(void);
Bool Pms_FwIsValid(void);
void Pms_ResetBatInfo(void);
Bool Battery_SerialNumIsSame(const uint8* pSerialNum1, const uint8* pSerialNum12);
Bool Battery_isOk(uint8 portId);
void Pms_SetBatVerify(uint8 port0, uint8 port1);
uint8 Pms_GetAveBatSoc(const BatStatePkt* pBatDesc);
Bool Pms_IsNotDischarge(void);
Bool Pms_SendCmd(uint8 cmd, const void* pData, int len, uint8 maxReTxCount);
#ifdef CFG_CABIN_LOCK
void Pms_SetCabinLock(Bool isLock);
#endif
#ifdef CFG_WHEEL_LOCK
void Pms_SetLock(Bool isLock);
#endif
void Pms_RemoveAllSn(uint8 count);
void Pms_SendCmdDischarge(void);
void Pms_UpdateBatteryInfo(const BatteryDesc* pNewDesc, int count);
Bool Battery_isIn(uint8 port);
void Pms_SetBatInfo(const BatStatePkt* pNewPkt, int len);

extern BOOL pmsPortEventInit( void );
extern BOOL pmsPortEventPost( pmsEventType eEvent );
extern BOOL pmsPortEventGet( pmsEventType * eEvent );
extern int slave_rs485_cmd_param_changed_polling(void);

#ifdef CFG_REMOTE_ACC
void Pms_SetAcc(Bool isOn);
#endif
void Pms_WakeupEx(void);

#ifdef __cplusplus
}
#endif

#endif


