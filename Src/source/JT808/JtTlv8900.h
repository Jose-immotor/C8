
#ifndef  _JT_TLV_8900_H_
#define  _JT_TLV_8900_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "typedef.h"
#include "utp.h"
	
	typedef enum
	{
		TAG_ACTIVE = 1,
		TAG_SET_ACC_STATE = 2,
		TAG_SET_WHELL_LOCK = 3,
		TAG_SET_CABIN_LOCK = 4,
	}CMD8900_TLV;

	UTP_EVENT_RC JtTlv8900_proc(const uint8* data, int len);
	void JtTlv8900_init();


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


