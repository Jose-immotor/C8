
#include "workmode.h"
#include "Common.h"

static workmode g_workmode;
uint32 g_ActiveFlag;

void WorkMode_Dump()
{

}

void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive)
{
	if(isActive)
	{
		g_ActiveFlag |= af;
	}
	else
	{
		g_ActiveFlag &= ~af;
	}
}

static void workmode_fsm_active(workmode* pworkmode, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if((g_workmode.active_flag == 0)&&(g_workmode.first_flag == 0))
	{
		ObjList_start();
		g_workmode.active_flag = 1;
		
	}
}

static void workmode_fsm_sleep(workmode* pworkmode, uint8_t msgId, uint32_t param1, uint32_t param2)
{
	if(g_workmode.sleep_flag == 0)
	{
		ObjList_stop();
		g_workmode.sleep_flag = 1;
	}
	if(((!g_ActiveFlag)&&((g_Bat[0].bmsInfo.state&0x0300)==0x0000))||
		(SwTimer_isTimerOutEx(g_workmode.statusSwitchTicks,WORKMODE_FORCE_SLEEP_TIME)))
	{
		Printf("g_ActiveFlag= 0x%08x.\n",g_ActiveFlag);
		g_workmode.first_flag = 0;
		g_workmode.sleep_flag = 0;
		g_workmode.active_flag = 0;
		Printf("workmode go to sleep mode!\n");
		Mcu_PowerDown();
	}
}

//查找状态响应的处理函数
static WorkMode_FsmFn WorkMode_findStatusProcFun(WorkmodeOpStatus status)
{
	struct
	{
		WorkmodeOpStatus opStatus;
		WorkMode_FsmFn OpFun;
	}
	const static g_fsms[] =
	{
		{WM_ACTIVE		, workmode_fsm_active},
		{WM_SLEEP		, workmode_fsm_sleep},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_fsms); i++)
	{
		if (g_fsms[i].opStatus == g_workmode.opStatus) return  g_fsms[i].OpFun;
	}

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

void workmode_switchStatus(WorkmodeOpStatus newStatus)
{
	if (newStatus == g_workmode.opStatus) return;

	g_workmode.statusSwitchTicks = GET_TICKS();

	g_workmode.opStatus = newStatus;
	g_workmode.Fsm = WorkMode_findStatusProcFun(newStatus);
}

static void workmode_fsm(uint8_t msgId, uint32_t param1, uint32_t param2)
{
	g_workmode.Fsm(msgId, param1, param2);
}

void WorkMode_run()
{
	workmode_fsm(0,0,0);
}

void WorkMode_start()
{
	workmode_switchStatus(WM_ACTIVE);
}

void WorkMode_init()
{
	const static Obj obj = {
	.name = "WorkMode",
	.Start = WorkMode_start,
	.Run = WorkMode_run,
	};

	ObjList_add(&obj);
	
	g_workmode.first_flag =1;
	g_workmode.active_flag =0;
	g_workmode.sleep_flag =0;
}
