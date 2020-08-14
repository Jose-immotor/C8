#ifndef __MSG_DEF_H_
#define __MSG_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum _MSG_ID
{
	MSG_TIMEOUT = 1,	//MSG_TIMEOUT(pTimer, timerID)

	MSG_RUN,			//MSG_RUN(0, 0)
	MSG_UTP_REQ_DONE,	//MSG_RUN(pUtpCmd, UTP_TXF_EVENT)

	MSG_MAX
}MSG_ID;

#ifdef __cplusplus
}
#endif

#endif

