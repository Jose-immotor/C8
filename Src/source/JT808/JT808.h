
#ifndef  _JT808_H_
#define  _JT808_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "HwFwVer.h"

	typedef enum
	{
		JTCMD_SIM_HB		= 0x01,
		JTCMD_MCU_HB		= 0x02,
		JTCMD_SET_OP_STATE	= 0x03,

		JTCMD_CMD_GET_SIM_ID  = 0x11,
		JTCMD_CMD_GET_SIM_CFG,
		JTCMD_CMD_GET_BLE_ID,
		JTCMD_CMD_GET_BLE_CFG,

		JTCMD_CMD_GET_FILE_INFO ,

		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,
	}JT_cmd;

	typedef enum
	{
		JT_STATE_INIT			= 0,
		JT_STATE_SLEEP			= 0x01,
		JT_STATE_WAKEUP			= 0x02,
		JT_STATE_PREOPERATION	= 0x04,
		JT_STATE_OPERATION		= 0x05,
	}JT_state;

	typedef struct _JT_devState
	{
		uint8_t cnt;
		uint8_t csq;
		uint8_t snr;
		uint8_t siv;
	}JT_devState;


	typedef struct _SimID
	{
		//传输协议版本号
		uint8_t protocolVer;
		HwFwVer ver;
		uint8_t SN[12];
	}SimID;

	typedef struct _JT808
	{
		//当前JT808的操作状态
		JT_state opState;
		//设置JT808的操作状态
		JT_state setToOpState;

		SimID simID;

		JT_devState devState;

		//是否定位成功
		Bool isLocation;

		uint16_t bleEnCtrl;
	}JT808;

	extern JT808* g_pJt;

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


