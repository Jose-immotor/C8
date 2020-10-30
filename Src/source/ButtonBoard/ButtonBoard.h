
#ifndef __BUTTONBOARD_H__
#define __BUTTONBOARD_H__

#include "typedef.h"

	typedef enum
	{
		BUCMD_SIM_HB		= 0x01,
		BUCMD_MCU_HB		= 0x02,
//		JTCMD_SET_OP_STATE	= 0x03,

//		JTCMD_CMD_GET_SIM_ID  = 0x11,
//		JTCMD_CMD_GET_SIM_CFG,
//		JTCMD_CMD_SET_SIM_CFG,
//		JTCMD_CMD_GET_SMS,
//		JTCMD_CMD_SEND_TO_SVR = 0x15,

//		JTCMD_CMD_GET_FILE_INFO = 0X16,

//		JTCMD_CMD_GET_FILE_CONTENT = 0X17,
//		JTCMD_CMD_SET_LOCATION_EXTRAS = 0x18,

//		JTCMD_CMD_GET_BLE_ID = 0x30,
//		JTCMD_CMD_GET_BLE_CFG,
//		JTCMD_CMD_SET_BLE_CFG,
//		JTCMD_CMD_GET_BLE_EN,
//		JTCMD_CMD_SET_BLE_EN,
//		
//		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,
//		JTCMD_EVENT_DEV_STATE_LOCATION,

//		JTCMD_BLE_EVT_AUTH = 0x90,
//		JTCMD_BLE_EVT_CNT = 0x91,
//		JTCMD_BLE_EVT_BEACON = 0x92,
//		JTCMD_BLE_RCV_DAT = 0x93,

//		JTCMD_EVT_RCV_SVR_DATA =  0xA0,
//		JTCMD_EVT_RCV_FILE_DATA ,
	}BU_cmd;
	
	typedef enum
	{
		BU_STATE_INIT			= 0x01,
		BU_STATE_SLEEP			,//= 0x02,
		BU_STATE_WAKEUP			,//= 0x03,
		BU_STATE_PREOPERATION	,//= 0x04,
		BU_STATE_OPERATION		,//= 0x05,

		BU_STATE_UNKNOWN		= 0xFF,
	}BU_state;
	typedef void (*ButtfsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
typedef struct _Butt
{
	BU_state opState;
	
	Bool online_sta;// «∑Ò‘⁄œﬂ
	
	ButtfsmFn fsm;	
}Butt;

void Butt_rxDataProc(const uint8_t* pData, int len);
void Button_init();

#endif
