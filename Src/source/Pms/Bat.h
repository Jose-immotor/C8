
#ifndef __BAT__H_
#define __BAT__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

	//��ؽṹ��
	typedef struct _Bat
	{
		uint8_t  port;			//��λ�ţ���0��ʼ
		uint8_t  isPresent;		//����Ƿ����
		uint32_t soc;			//��ص���

		uint32_t actChgMA;		//ʵ�ʵĳ�����(�ӵ�ض�������ʵ�ʳ�����)
		uint32_t allocChgMA;	//��ǰ���������
		uint32_t newPwrMA;		//��������µĳ�����
	}Bat;

#ifdef __cplusplus
}
#endif

#endif


