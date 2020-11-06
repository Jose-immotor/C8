
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
		uint8 Display_content[3];//��ʾ������ʾ���ݣ�ȡֵ��Χ[0,9]����λ�����ȷ���
		uint8 Display_luminance;//��ʾ���ȣ�ȡֵ��Χ[0,8]
		uint16 Display_time;//��λ����;��λ�����ȷ���

	}BU_DisplayCfg;
	
	typedef void (*ButtfsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
typedef struct _Butt
{
	Bool online_sta;//�Ƿ�����
	
	union
	{
		uint8 State_Parameter;//
		struct
		{
			uint8 isStateWork : 1;//BIT0������0ʱ����Ϊ˯��ģʽ;����1ʱ����Ϊ����ģʽ
			uint16 reserved01 : 7;//BIT[1-7]:����
		};
	};
	BU_DisplayCfg DisplayCfg;
	
	BU_state opState;
	ButtfsmFn fsm;	
}Butt;

void Butt_rxDataProc(const uint8_t* pData, int len);
void Button_init();

#endif
