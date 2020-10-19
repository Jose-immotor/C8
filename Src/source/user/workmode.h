
#ifndef __WORKMODE_H__
#define __WORKMODE_H__

#include "Bit.h"
#include "Message.h"

#define WORKMODE_FORCE_SLEEP_TIME 5*1000//5��

	//���־���壬���ñ�־����λʱ���豸���ܽ���˯�ߡ�
	typedef enum _ActiveFlag
	{
		 AF_MDB = BIT_0			//modbus���ڹ���״̬
		,AF_NVC = BIT_1			//NVC���ڲ�������
		,AF_CABIN = BIT_2		//���������ڹ���״̬
		,AF_PMS = BIT_3			//PMS���ڹ���״̬
		,AF_NFC = BIT_4			//NFC���ڹ���״̬
	//	
	//	,AF_PMS = BIT_16			//PMS
	//	,AF_BEACON = BIT_17			//BEACON�Ƿ�����״̬
	//	
	//	,AF_FW_UPGRADE = BIT_24	//����������������
	}ActiveFlag;

	//work״̬
	typedef enum _WorkmodeOpStatus
	{
		WM_ACTIVE = 1,	//����
		WM_SLEEP	,	//����
	}WorkmodeOpStatus;
	
	typedef void (*WorkMode_FsmFn)(uint8_t msgId, uint32_t param1, uint32_t param2);
	typedef struct _workmode
	{
		//workmode����״̬
		WorkmodeOpStatus opStatus;
		//״̬������ָ�룬��status����ָ���Ǹ�״̬������
		WorkMode_FsmFn Fsm;
		
		uint32_t statusSwitchTicks;
		
		uint8_t first_flag;
		uint8_t active_flag;
		uint8_t sleep_flag;

	}workmode;


void WorkMode_init();
void workmode_switchStatus(WorkmodeOpStatus newStatus);
void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);	
#endif
