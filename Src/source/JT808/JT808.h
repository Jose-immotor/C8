
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
		JTCMD_CMD_SEND_TO_SVR = 0x15,

		JTCMD_CMD_GET_FILE_INFO ,

		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,

		JTCMD_EVT_RCV_SVR_DATA =  0xA0,
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

	typedef struct _JT_devState
	{
		uint8_t cnt;
		uint8_t csq;
		uint8_t snr;
		uint8_t siv;
	}JT_devState;


	typedef struct _SimID
	{
		//����Э��汾��
		uint8_t protocolVer;
		HwFwVer ver;
		uint8_t SN[12];
	}SimID;

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//��ǰJT808�Ĳ���״̬
		JT_state opState;
		//����JT808�Ĳ���״̬
		JT_state setToOpState;

		SimID simID;

		JT_devState devState;

		//�Ƿ�λ�ɹ�
		Bool isLocation;

		uint16_t bleEnCtrl;

		JT808fsmFn fsm;
	}JT808;

	extern JT808* g_pJt;
	void JT808_init();

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


