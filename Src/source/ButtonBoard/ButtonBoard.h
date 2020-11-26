
#ifndef __BUTTONBOARD_H__
#define __BUTTONBOARD_H__

#include "typedef.h"

	typedef enum
	{
		BUCMD_BUTT_HB		= 0x01,
		BUCMD_MCU_HB		= 0x02,
		BUCMD_SET_OP_STATE	= 0x03,

		BUTTCMD_SET_DISPLAYCFG  = 0x20,
		BUTTCMD_GET_DISPLAYCFG  = 0x30,

	}BU_cmd;
	
	typedef enum
	{
		BU_STATE_INIT			,
		BU_STATE_SLEEP			,//= 0x02,
		BU_STATE_WAKEUP			,//= 0x03,
		BU_STATE_PREOPERATION	= 0x04,
		BU_STATE_OPERATION		= 0x05,

		BU_STATE_UNKNOWN		= 0xFF,
	}BU_state;
	
	typedef struct
	{
		uint8 Display_content[3];//显示内容显示内容：取值范围[0,9]；高位数据先发送
		uint8 Display_luminance;//显示亮度：取值范围[0,8]
		uint16 Display_time;//单位：秒;高位数据先发送

	}BU_DisplayCfg;
	
	typedef void (*ButtfsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
typedef struct _Butt
{
	Bool online_sta;//是否在线
	
	union
	{
		uint8 State_Parameter;//
		struct
		{
			uint8 isStateWork : 1;//BIT0：等于0时设置为睡眠模式;等于1时设置为唤醒模式
			uint16 reserved01 : 7;//BIT[1-7]:保留
		};
	};
	BU_DisplayCfg DisplayCfg;
	
	BU_state opState;
	ButtfsmFn fsm;	
}Butt;

void Butt_rxDataProc(const uint8_t* pData, int len);
void Button_init();

#endif
