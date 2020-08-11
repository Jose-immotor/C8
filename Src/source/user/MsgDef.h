#ifndef __MSG_DEF_H_
#define __MSG_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum _MSG_ID
{
	MSG_TIMEOUT = 1,	//消息格式：(MSG_TIMEOUT, pTimer, timerID)

	MSG_RUN,
	MSG_UTP_REQ_DONE,

	MSG_MAX
}MSG_ID;

#ifdef __cplusplus
}
#endif

#endif

