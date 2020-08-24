#ifndef __PMS_LOG_DEF_H_
#define __PMS_LOG_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "Log.h"
#include "PmsPeripheralDef.h"
	/*****************************************
	PMS设备和外设ID定义
	****************************************/

#define STR_PMS_STATE1(spac) 		\
		"B[0]:%0B{1-batIn|0-batOut}"spac		\
		"B[1]:%1B,chgrCnt"spac		\
		"B[2]:%2B{0-doorOpen|1-doorClosed}"spac		\
		"B[3-4]:%3-4B{0-mid|1-Up|2-Down|3-Up&Down},batLock"spac	\
		"B[5]:%5B;doorRun"spac	\
		"B[6]:%6B;funOn"spac	\
		"B[7]:%7B{0-NoFire|1-Fire******************}"

#define STR_PMS_STATE2(spac) 			\
		"B[0]:%0B,chgrEn"spac 			\
		"B[1]:%1B,chgrPwrOn"spac		\
		"B[2]:%2B,heatRSwhOn"spac	\
		"B[3]:%3B,BpPwrOn"spac			\
		"B[4]:%4B,pmsAuth"spac			\
		"B[5]:%5B,batAuth"spac			\
		"B[6]:%6B,heatREn" 

#define STR_PMS_STATE3(spac) 			\
		"B[0]:%0B,fuseOK"spac		\
		"B[1]:%1B,heatTemp"spac			\
		"B[2]:%2B,smk2Up0.6"spac	\
		"B[3]:%3B,smk10Up1.2"spac	\
		"B[4]:%4B,smk20SOn2.7"spac	\
		"B[5]:%5B,bmsInBL"spac			\
		"B[6]:%6B,chgrID"spac			\
		"B[7]:%7B,chgrIDOut"	

#define STR_PMS_STATE4(spac) 			\
		"B[0]:%0B,batSwhDown"spac		\
		"B[1]:%1B,batIsIn"spac		\
		"B[2]:%2B,pmsTrs"spac		\
		"B[3]:%3B,chgrFwUpg"spac	\
		"B[4]:%4B;bmsFwUpg"spac	\
		"B[5]:%5B;nfcState"spac	


#define STR_PMS_CMD(spac) 			\
		"B[0]:%0B,unlock"spac 			\
		"B[1]:%1B,ChgrEn"spac			\
		"B[2]:%2B,ChgTmp"spac			\
		"B[5]:%3B,verfEn"spac			\
		"B[6]:%6B,capEn"spac		\
		"B[7]:%7B,heatEn"spac			\
		"B[8-9]:%8-9B{0-Unk|1-220V|2-48V|3-18650}"spac		\
		"B[10]:%10B,disChgEn"spac		\
		"B[12]:%10B,funOn"spac			\
		"B[13]:%10B,glaEn"spac		\
		"B[14]:%10B,actDetEn"		

//OD_pms_detected_err_of_bat
#define STR_ACT_DETECT_BAT_ERR(spac) 	\
		"B[0]:%0B,magnetF"spac			\
		"B[1]:%1B,mosFault"spac			\
		"B[2]:%2B,preDischgFault"spac	\
		"B[3]:%3B,disChgFault"spac		\
		"B[4]:%4B,chgMosShort"spac		\
		"B[5]:%5B,chgMosOpen"	

#define STR_PMS_DETECT_BAT_ERR_0_7(spac) 	\
		"B[0]:%0B,chgrI-batI>600"spac		\
		"B[1]:%1B,chg-TempHigh"spac			\
		"B[2]:%2B,batPreStartFaild"spac		\
		"B[4]:%4B,chgr_bat_vol_Fault"spac	\
		"B[5]:%5B,batFullCurrentFault"spac	\
		"B[6]:%6B,batCurOutFault"spac		\
		"B[7]:%7B,batSoc0_V_Fault"spac		

#define STR_PMS_DETECT_BAT_ERR_8_13(spac) 	\
		"B[8]:%8B,UVP_SOC_fault"spac		\
		"B[9]:%9B,UVP&OVP"spac				\
		"B[10]:%10B,UTP&OTP"spac			\
		"B[11]:%11B,hallNotDetMagnet"spac	\
		"B[12]:%12B,cellVollow"spac			\
		"B[13]:%13B,chgrDetOCP"spac			

#define STR_PMS_DETECT_BAT_ERR_14_17(spac) 	\
		"B[14]:%14B,chgrDetShort"spac		\
		"B[15]:%15B,cellsVolBigDiff"spac	\
		"B[16]:%16B,chgrDetOVP"spac			\
		"B[17]:%17B,chgrDetOTP"				

#define STR_PMS_OTHER_ERR(spac) 			\
		"B[0]:%0B,openDoorFaild"spac		\
		"B[1]:%1B,chgNoCurrent"spac			\
		"B[2]:%2B,heatFault"		

#define STR_PMS_FAULT(spac) 				\
		"B[0]:%0B,OVP"spac					\
		"B[1]:%1B,UVP"spac					\
		"B[2]:%2B,OCP"spac					\
		"B[3]:%3B,OTP"spac					\
		"B[4]:%4B,UTP"spac					\
		"B[5]:%5B,OthersFault"spac			\
		"B[6]:%6B,doorNotExpectOpen"spac	\
		"B[7]:%7B,chgrFault"		

#define STR_PMS_DAMAGE(spac) 			\
		"B[0]:%0B,unpack"spac			\
		"B[1]:%1B,crash"spac			\
		"B[2]:%2B,forbitChg"spac		\
		"B[5]:%5B,disChgCntFault"spac	\
		"B[6]:%6B,chgFault"					

#define STR_BMS_REG_OPFT1_0_7(spac)			\
		"B[0]:%0B,OVP"spac				\
		"B[1]:%1B,UVP1"spac				\
		"B[2]:%2B,UVP2"spac				\
		"B[3]:%3B,CHG_OCP"spac			\
		"B[4]:%4B,DIS_OCP1"spac			\
		"B[5]:%5B,DIS_OCP2"spac			\
		"B[6]:%6B,CHG_OTP"spac			\
		"B[7]:%7B,DIS_OTP"spac			

#define STR_BMS_REG_OPFT1_8_15(spac)	\
		"B[8]:%8B,CHG_UTP"spac			\
		"B[9]:%9B,DIS_UTP"spac			\
		"B[10]:%10B,CHG_MOS_OTP"spac	\
		"B[11]:%11B,DIS_MOS_OTP"spac	\
		"B[12]:%12B,CONN_OTP"spac		\
		"B[14]:%14B,PRE_CHG_OT"spac		\
		"B[15]:%15B,PRE_CHG_OC"

#define STR_BMS_REG_OPFT2_0_7(spac)			\
		"B[0]:%0B,DIS_SCP"spac				\
		"B[1]:%1B,DIS_OCP_LOCK"spac			\
		"B[2]:%2B,CHG_OCP_LOCK"spac			\
		"B[3]:%3B,AFE_ALERT"spac			\
		"B[4]:%4B,AFE_XREADY"spac			\
		"B[5]:%5B,SLEEP_OCP"spac			\
		"B[7]:%7B,TVS_OTP"spac				\

#define STR_BMS_REG_OPFT2_8_15(spac)	\
		"B[8-9]:%8-9B{0-None|1-250|2-500|3-1000},ILLEGAL_CHG_I"spac				\
		"B[10]:%10B,ILLEGAL_CHG_V"spac				\
		"B[11]:%11B,T_RISE_FAST"		

//
#define STR_CAPA_TEST_RESULT(spac)	\
		"B[0]:%0B{0-None|1-chgrFrr}"spac		\
		"B[1]:%1B{0-None|1-disChgErr}"spac		\
		"B[2]:%2B{0-None|1-openDoor}"spac		\
		"B[3]:%3B{0-None|1-stopCmd}"spac		\
		"B[4]:%4B{0-None|1-smkAlarm}"spac		\
		"B[5]:%5B{0-None|1-noBat}"spac		\
		"B[6]:%6B{0-None|1-batErr}"spac			\
		"B[7]:%7B{0-None|1-batCntErr}"spac		\
		"B[8]:%8B{0-None|1-chgDisEn}"spac	
//
#define STR_CAPA_TEST_STATE()	\
		"state:%0-15B{"\
		"0-init|"\
		"1-wait|"\
		"2-rcvCmd|"\
		"3-disChgDelay|"\
		"4-disChg|"\
		"5-disChgErrProc|"\
		"6-chgBeforeDelay|"\
		"7-chg|"\
		"8-testErr|"\
		"9-testDone}"\
		", SOC:%16-31B"		

	//PMS事件类型ID定义
	typedef enum _PmsCatID
	{
		//system event
		PMS_CATID_SYS = 1,		//PMS系统事件类型ID
		PMS_CATID_PERIPHERAL,	//PMS外设事件类型ID
		PMS_CATID_FW_UPGRADE,	//PMS升级事件类型ID
	}PmsCatID;

	typedef enum _PmsSubID
	{
		//system event
		PMS_SBUID_COMMON = 0,	//PMS系统事件类型ID
	}PmsSubID;

#define ET_PMS_DISCHG_START  0	//放电启动
#define ET_PMS_DISCHG_RESULT 1	//放电结束

	typedef enum _PmsLogEventID
	{
		//system event
		ET_PMS_RESET = 1,			//PMS复位，LOG_TRACE1(PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_RESET, 0)
		ET_PMS_OP_STATE_CHANGE,		//PMS操作状态, 格式 ET_PMS_OP_STATE_CHANGE(PMS_CATID_VER, 0, ET_PMS_CMD_STATE, newState)

		ET_PMS_CMD_STATE = 0x20,	//PMS控制状态, 格式 LOG_TRACE1(PMS_CATID_VER, 0, ET_PMS_CMD_STATE, value)

		ET_PMS_BAT_PRESENT_CHANGED,	//ET_PMS_BAT_PRESENT_CHANGED , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_PRESENT_CHANGED ,PMS_STATE1)
		ET_PMS_DOOR_STATE_CHANGED,	//ET_PMS_DOOR_STATE_CHANGED  , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_DOOR_STATE_CHANGED  ,PMS_STATE1)
		ET_PMS_BAT_LOCK_CHANGED,	//ET_PMS_BAT_LOCK_CHANGED    , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_LOCK_CHANGED    ,PMS_STATE1)
		ET_PMS_FIRE_ALARM,			//ET_PMS_FIRE_ALARM  , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_LOCK_CHANGED    ,PMS_STATE1,PMS_STATE2,PMS_STATE3,PMS_STATE4)
		ET_PMS_FIRE_ALARM1,			//ET_PMS_FIRE_ALARM1 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_LOCK_CHANGED    ,METER_TEMP|DISCHG_MOS_TEMP|CELL1_TEMPCELL2_TEMP)
		ET_PMS_FIRE_ALARM2,			//ET_PMS_FIRE_ALARM2 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_LOCK_CHANGED    ,PMS_STATE1)
		ET_PMS_FIRE_ALARM3,			//ET_PMS_FIRE_ALARM3 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON , ET_PMS_BAT_LOCK_CHANGED    ,PMS_STATE1)

		ET_PMS_CHG_ENABLE_CHANGED,	//PMS STATE2 , 格式 LOG_TRACE1(PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CHG_ENABLE_CHANGED, ET_PMS_STATE2)
		ET_PMS_STATE1,				//PMS ET_PMS_STATE1 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_STATE1, state1)
		ET_PMS_STATE2,				//PMS ET_PMS_STATE2 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_STATE2, state2)
		ET_PMS_STATE3,				//PMS ET_PMS_STATE3 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_STATE3, state3)
		ET_PMS_STATE4,				//PMS ET_PMS_STATE4 , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_STATE4, state4)

		ET_PMS_CAP_TEST_STATE,		//PMS容量测试, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CAP_TEST_STATE, CancelReason,SOC)
		ET_PMS_CAP_TEST_RESULT,		//PMS容量测试, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CAP_TEST_RESULT, capTest_State,SOC)
		ET_PMS_CAP_TEST_VALUE,		//PMS容量    , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CAP_TEST_VALUE, Capacity,SOC)
		ET_PMS_CAP_TEST_TIME,		//PMS测试时间, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CAP_TEST_VALUE, costSeconds)
		ET_PMS_FIRE,				//PMS火警    , 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_FIRE, SOC, ON/OFF)
		ET_PMS_DISCHG,				//PMS放电事件, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_DISCHG, SOC, DisChgEn/DisChgDisable, 0-start|1-result, isError)
		ET_PMS_CHG,					//PMS充电事件, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_PMS_CHG   , SOC, DisChgEn/DisChgDisable, 0-start|1-result, isError)
		ET_SOC_CHANGED,				//SOC改变事件, 格式 (PMS_CATID_SYS, PMS_SBUID_COMMON, ET_SOC_CHANGED, SOC)

		ET_PMS_BP_START = 0x40,		//备电系统启动事件, 格式 (ET_PMS_BP_START)
		ET_PMS_BP_STOP,				//备电系统终止事件, 格式 (ET_PMS_BP_STOP, BpEventType(reason))
	}PmsLogEventID;

	const char* Pms_ModuleValueToStr(const LogItem* pItem, char* buf, int bufSize);
	const char* Pms_EventIdToStr(const LogItem* pItem, char* buf, int bufSize);
	const char* Pms_EventValueToStr(const LogItem* pItem, char* buf, int bufSize);


#ifdef __cplusplus				   
}								   
#endif							   
								   
#endif							   

































































