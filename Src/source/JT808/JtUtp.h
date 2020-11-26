#ifndef __UTP_BASE_H_
#define __UTP_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

	//UTP �����붨��
	typedef enum _OP_CODE
	{
		OP_SUCCESS = 0
		,OP_PARAM_INVALID	= 1 //�ò�����Ч
		,OP_UNSUPPORTED = 2 //�����֧��
		,OP_CRC_ERROR		= 3	//У�����
		,OP_NOT_READY		= 4	//�豸û׼����
		,OP_USERID_ERROR	= 5	//USER_ID����
		,OP_HW_ERROR		= 6	//ִ��ʧ��
		,OP_18650BAT_V_OVER_LOW	= 7	//18650��ѹ̫��
		,OP_NOT_ALLOW_DISCHARGE	= 8	//��֧�ַŵ�
		
		,OP_PENDING			= 0xF0	//ִ�еȴ�
		,OP_NO_RSP = 0xF1	//No response
		,OP_FAILED			= 0xFF	//ִ��ʧ��
	}OP_CODE;

	typedef struct _JtUtp
	{
		uint8_t  checkSum;
		uint8_t  ver;
		uint16_t vendor;
		uint8_t  devType;
		uint8_t  cmd;
		uint8_t  len;
		uint8_t  data[1];
	}JtUtp;

	extern const UtpFrameCfg g_jtFrameCfg;
#ifdef __cplusplus
}
#endif

#endif


