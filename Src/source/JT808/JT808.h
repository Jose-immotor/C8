
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
		JTCMD_CMD_GET_BLE_ID,
		JTCMD_CMD_GET_BLE_CFG,
		JTCMD_CMD_SEND_TO_SVR = 0x15,

		JTCMD_CMD_GET_FILE_INFO ,

		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,

		JTCMD_BLE_EVT_AUTH = 0x90,
		JTCMD_BLE_EVT_CNT = 0x91,
		JTCMD_BLE_RCV_DAT = 0x93,

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

#pragma pack(1)

	//�ն˵��豸���Զ���
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint8  protocolVer;	//����Э��汾��
		uint32 devClass;	//�ն�����
		uint8  vendorID[5];	//������ID
		uint8  devModel[20];//�ն��ͺ�
		uint8  devId[12];	//�ն� ID
		uint8  iccid[10];	//�ն� SIM �� ICCID
		uint8  hwVer[JT_DEV_HW_VER_SIZE];	//�ն�Ӳ���汾��,��0��β
		uint8  fwVer[JT_DEV_FW_VER_SIZE];	//�ն˹̼��汾��,��0��β
	}JtDevProperty;
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//��ǰJT808�Ĳ���״̬
		JT_state opState;
		//����JT808�Ĳ���״̬
		JT_state setToOpState;

		JT_devState devState;

		//�Ƿ�λ�ɹ�
		Bool isLocation;

		uint16_t bleEnCtrl;

		//�ն���������
		JtDevProperty property;
		JtDevCfgParam cfgParam;

		JT808fsmFn fsm;

	}JT808;

	extern JT808* g_pJt;
	extern JT808 g_Jt;
	void JT808_init();

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


