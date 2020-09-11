
#ifndef __BATTERY__H_
#define __BATTERY__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "BmsReg.h"
#include "ModBus.h"
#include "NfcCardReader.h"

#define BMS_INQUERY_INTERVAL_MS 1000	//轮询事件间隔
#define NFC_MAX_TRANS_SIZE 		125		//NFC每次传输的最大包长度

#define BMS_REG_ID_SIZE  sizeof(BmsReg_info)
#define BMS_REG_ID_COUNT (sizeof(BmsReg_info) / 2)

#define BMS_REG_INFO_COUNT_1 ((BMS_REG_INFO_ADDR_2 - BMS_REG_INFO_ADDR_1))
#define BMS_REG_INFO_SIZE_1  (BMS_REG_INFO_COUNT_1 * 2)
#define BMS_REG_INFO_COUNT_2 (BMS_REG_INFO_END - BMS_REG_INFO_ADDR_2 + 1)
#define BMS_REG_INFO_SIZE_2  (BMS_REG_INFO_COUNT_2 * 2)

#define BMS_REG_CTRL_SIZE  sizeof(BmsReg_ctrl)
#define BMS_REG_CTRL_COUNT (sizeof(BmsReg_ctrl) / 2)

	/*
	typedef enum _BmsCmdCtrl
	{
		RegCtrl_chg_en = BIT_0,
		RegCtrl_chg_disEn  = BIT_1,
		RegCtrl_disChg_en  = BIT_2,
		RegCtrl_disChg_disEn = BIT_3,
		RegCtrl_preDischg_en 	= BIT_4,
		RegCtrl_preDischg_disEn  = BIT_5,
		RegCtrl_sleep_en 		= BIT_6,
		RegCtrl_sleep_disEn  	= BIT_7,
		RegCtrl_deepSleep_en 	= BIT_8,
		RegCtrl_deepSleep_disEn = BIT_9,
	}BmsCmdCtrl;
	
*/
	typedef enum _BmsCmd
	{
		BMS_READ_ID = 0X01,	//读BMS身份版本寄存器
		BMS_READ_INFO_1,	//读BMS信息寄存器1, 一个包太大，分开2次读取，
		BMS_READ_INFO_2,	//读BMS信息寄存器2
		BMS_READ_CTRL,		//读BMS控制寄存器

		BMS_WRITE_CTRL, 	//写控制寄存器
	}BmsCmd;

	//电池消息定义
	typedef enum _BmsMsg
	{
		BmsMsg_run = 1,		//运行，消息格式：(BmsMsg_run, 0, 0)
		BmsMsg_timeout,		//超时，消息格式：(BmsMsg_timeout, timerID, 0)
		BmsMsg_cmdDone,		//命令结束，消息格式：(BmsMsg_cmdDone, cmd, MOD_EVENT_RC)

		BmsMsg_batPlugIn,	//电池插入，消息格式：(BmsMsg_batPlugIn, 0, 0)
		BmsMsg_batPlugout,	//电池拔出，消息格式：(BmsMsg_batPlugout(0, 0)
		BmsMsg_active,		//电池激活，消息格式：(BmsMsg_active, ctrl, 0)
		BmsMsg_deactive,	//电池去激活，消息格式：(BmsMsg_deactive, 0, 0)

		BmsMsg_sleep,		//电池浅休眠，消息格式：(BmsMsg_sleep, 0, 0)
		BmsMsg_deepSleep,	//电池深休眠，消息格式：(BmsMsg_deepSleep, 0, 0)
		BmsMsg_wakeup,		//电池唤醒，消息格式：(BmsMsg_wakeup, 0, 0)
	}BmsMsg;

	//电池操作状态
	typedef enum _BmsOpStatus
	{
		BmsStatus_init,		//初始状态
		BmsStatus_readInfo,	//轮询电池，完毕切换到Idle状态
		BmsStatus_idle,		//空闲，等待下一次轮询
		BmsStatus_sleep,	//浅休眠，轮询电池状态
		BmsStatus_deepSleep,//深度休眠，不轮询电池状态
	}BmsOpStatus;

	//电池在位状态
	typedef enum _BatPresentStatus
	{
		BAT_UNKNOWN,	//不知道是否在位（没检测完毕）
		BAT_NOT_IN,	//电池不在位
		BAT_IN,		//电池在位，（获取所有信息完毕）
	}BatPresentStatus;

	typedef struct _Battery
	{
		NfcCardReader cardReader;

		uint8_t  port;					//电池端口好，从0开始
		uint32_t idleTicks;				//去激活时间Ticks
		uint32_t statusSwitchTicks;		//状态切换Ticks
		Bool 	 isActive;				//是否激活

		Bool 	 isReadyFroInquery;	//是否准备好查询电池

		BatPresentStatus presentStatus;	//电池在位状态
		BmsOpStatus 	 opStatus;	//电池操作状态

		//static data

		BmsReg_info 		bmsID;		//信息寄存器
		BmsReg_deviceInfo 	bmsInfo;	//只读寄存器
		BmsReg_ctrl 		bmsCtrl;	//控制寄存器

		//需要写到BMS寄存器BmsRegCtrl的值
		uint16_t   			writeBmsCtrl;

		//bmsCtrl镜像变量,记录WIRTE成功时的值。
		//仅供Modbos传输协议使用，其他模块不能修改
		uint16_t 			writeBmsCtrl_mirror;
		
		BmsRegCtrl* 		pBmsReg_Ctrl;	//初始化指向 writeBmsCtrl;
		const ModCfg* cfg;
	}Battery;

	void Bat_init(Battery* pBat, int port, const ModCfg* cfg);
	void Bat_start(Battery* pBat);
	void Bat_run(Battery* pBat);
	const uint8* Bat_getBID(Battery* pBat);
	void Bat_dump(const Battery* pBat);
	void Bat_bmsInfoDump(const Battery* pBat);

	//电池信息有效
	Bool Bat_isReady(Battery* pBat);

	//消息函数，
	void Bat_msg(Battery* pBat, uint8_t msgId, uint32_t param1, uint32_t param2);

	//电池控制函数
	void Bat_setPreDischg(Battery* pBat, Bool en);
	void Bat_setDischg(Battery* pBat, Bool en);
	void Bat_setChg(Battery* pBat, Bool en);
	void Bat_setSleep(Battery* pBat, Bool en);
	void Bat_setDeepSleep(Battery* pBat, Bool en);


	//Bat 事件函数
	MOD_EVENT_RC Bat_event(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev);
	MOD_EVENT_RC Bat_event_readBmsInfo(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev);

#ifdef __cplusplus
}
#endif

#endif


