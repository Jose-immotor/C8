#ifndef __UTP_BASE_H_
#define __UTP_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

	//UTP 错误码定义
	typedef enum _OP_CODE
	{
		OP_SUCCESS = 0
		,OP_PARAM_INVALID	= 1 //该参数无效
		,OP_UNSUPPORTED = 2 //该命令不支持
		,OP_CRC_ERROR		= 3	//校验错误
		,OP_NOT_READY		= 4	//设备没准备好
		,OP_USERID_ERROR	= 5	//USER_ID错误
		,OP_HW_ERROR		= 6	//执行失败
		,OP_18650BAT_V_OVER_LOW	= 7	//18650电压太低
		,OP_NOT_ALLOW_DISCHARGE	= 8	//不支持放电
		
		,OP_PENDING			= 0xF0	//执行等待
		,OP_NO_RSP = 0xF1	//No response
		,OP_FAILED			= 0xFF	//执行失败
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


