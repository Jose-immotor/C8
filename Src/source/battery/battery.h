

/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef __APP_TASK_BAT_H__
#define __APP_TASK_BAT_H__

#include <rtthread.h>
#include <stdbool.h>
#include "nfc_cmd_process.h"
#include "fm175xx.h"
#include "typedef.h"


#ifndef BATTERY_PROCESS_C
#define GLOBAL_BPROC extern
#else
#define GLOBAL_BPROC
#endif



typedef enum{
    ENUM_DISCHARGE,
    ENUM_CHARGE,
    ENUM_SLEEP
}ENUM_BMS_STATE;

//霍尔传感器在线状态
extern unsigned char gl_hall_gpio_state;
extern unsigned char state_take_part;
enum{
    ENUM_NFC_CMD_INDEX_R_VERSION,
    ENUM_NFC_CMD_INDEX_R_INFO1,
    ENUM_NFC_CMD_INDEX_R_INFO2,
    ENUM_NFC_CMD_INDEX_R_CTRL,
    ENUM_NFC_CMD_INDEX_W_CTRL,
    ENUM_NFC_CMD_INDEX_R_PARAM1,
    ENUM_NFC_CMD_INDEX_R_PARAM2,
    ENUM_NFC_CMD_INDEX_W_PARAM,
    ENUM_NFC_CMD_INDEX_R_USERDATA,
    ENUM_NFC_CMD_INDEX_W_USERDATA,
    ENUM_NFC_CMD_INDEX_W_RESET,
    ENUM_NFC_CMD_INDEX_R_BLV,//查询bootloader版本号
    ENUM_NFC_CMD_INDEX_BYPASS,// 透传
    ENUM_NFC_CMD_INDEX_AUTHORITY, //授权命令
    ENUM_NFC_CMD_INDEX_HISTORY_RECORD,
    ENUM_NFC_CMD_INDEX_MAX
};

//用来标志哪些数据已经从BMS中读取完成，可以供参考使用
#define BMS_DEV_INFO_FLAG_BIT (1<<0)
#define BMS_READONLY_DATA1_FLAG_BIT (1<<1)
#define BMS_READONLY_DATA2_FLAG_BIT (1<<2)
#define BMS_CTRL_FLAG_BIT (1<<3)
#define BMS_PARAM1_FLAG_BIT (1<<4)
#define BMS_PARAM2_FLAG_BIT (1<<5)
#define BMS_USER_FLAG_BIT (1<<6) 



// 非零表示有命令要发送
GLOBAL_BPROC unsigned char gl_cmd_buf[NFC_READER_COUNT_MAX][ENUM_NFC_CMD_INDEX_MAX];



#define BMS_CTRL_BIT_CHG_SWITCH (1<<0)
#define BMS_CTRL_BIT_SUPPLY_SWITCH (1<<1)
#define BMS_CTRL_BIT_PRESUPPLY_SWITCH (1<<2)

#define BMS_STATE_BIT_CHG_SWITCH (1<<0)
#define BMS_STATE_BIT_SUPPLY_SWITCH (1<<1)
#define BMS_STATE_BIT_PRESUPPLY_SWITCH (1<<2)
//充电器接入位
#define BMS_STATE_BIT_CHG_IN (1<<7)
//电池充满电标志位
#define BMS_STATE_BIT_FULLY_CHARGE (1<<14)






enum{
ENUM_REG_ADDR_PRVER = 0, //协议版本
ENUM_REG_ADDR_HWVER = 1, //硬件版本
ENUM_REG_ADDR_BLVER = 2, //Bootloader版本
ENUM_REG_ADDR_FWMSV = 3, //固件版本
ENUM_REG_ADDR_FWREV = 4, //固件版本- 修正版本
ENUM_REG_ADDR_FWBNH = 5, //编译版本，高
ENUM_REG_ADDR_FWBNL = 6, //编译版本，低
    
ENUM_REG_ADDR_MCUN12 = 7,
ENUM_REG_ADDR_MCUN34 = 8,
ENUM_REG_ADDR_MCUN56 = 9,
ENUM_REG_ADDR_MCUN78 = 10,

ENUM_REG_ADDR_BVOLT = 12, //额定电压
ENUM_REG_ADDR_BCAP = 13,//额定容量
ENUM_REG_ADDR_SN12 = 14,
ENUM_REG_ADDR_SN34 = 15,
ENUM_REG_ADDR_SN56 = 16,
ENUM_REG_ADDR_SN78 = 17,
ENUM_REG_ADDR_ERECH = 18, // 最早记录号高字
ENUM_REG_ADDR_ERECL = 19, // 最早记录号低字
ENUM_REG_ADDR_LRECH = 20, // 最近记录号高字
ENUM_REG_ADDR_LRECL = 21, // 最近记录号低字


ENUM_REG_ADDR_LTSTA = 22, //历史状态

//USER ID 
ENUM_REG_ADDR_RO_USERID12 = 256,
ENUM_REG_ADDR_RO_USERID34 = 257,
ENUM_REG_ADDR_RO_USERID56 = 258,
ENUM_REG_ADDR_RO_USERID78 = 259,

ENUM_REG_ADDR_STATE = 260,
ENUM_REG_ADDR_SOC = 261,
ENUM_REG_ADDR_TVOLT = 262, //目前输出电压
ENUM_REG_ADDR_TCURR = 263, //目前输出电流
ENUM_REG_ADDR_HTEMP = 264, //最高电池温度
ENUM_REG_ADDR_LTEMP = 266,//最低电池温度
ENUM_REG_ADDR_HVOLT = 268,//最高单体电压
ENUM_REG_ADDR_HVNUM = 269, // 最高单体电压电池编号
ENUM_REG_ADDR_LVOLT = 270,// 最低单体电压
ENUM_REG_ADDR_LVNUM = 271, // 最低单体电压电池编号
ENUM_REG_ADDR_DSOP = 272, // 10s最大允许放电电流
ENUM_REG_ADDR_CSOP = 273,//10s最大允许充电电流
ENUM_REG_ADDR_SOH = 274,//         健康状态
ENUM_REG_ADDR_CYCLE = 275,//循环次数

ENUM_REG_ADDR_DEVFT1 = 280, //设备故障字1
ENUM_REG_ADDR_DEVFT2 = 281, //设备故障字1
ENUM_REG_ADDR_OPFT1 = 282, //运行故障字1
ENUM_REG_ADDR_OPFT2 = 283, //运行故障字2
ENUM_REG_ADDR_OPWARN1 = 284, //运行告警字1
ENUM_REG_ADDR_OPWARN2 = 285, //运行告警字2
ENUM_REG_ADDR_CMOST = 286, //充电MOS温度
ENUM_REG_ADDR_DMOST = 287, //放电MOS温度
ENUM_REG_ADDR_FUELT = 288 , //电量计温度,
ENUM_REG_ADDR_CONT = 289, //连接器温度
ENUM_REG_ADDR_BTEMP1 = 290, //电池温度1
ENUM_REG_ADDR_BTEMP2 = 291, //电池温度2
ENUM_REG_ADDR_BVOLT1 = 292, // 单体电池1电压
ENUM_REG_ADDR_BVOLT2 = 293, // 单体电池2电压
ENUM_REG_ADDR_BVOLT3 = 294, // 单体电池3电压
ENUM_REG_ADDR_BVOLT4 = 295, // 单体电池4电压
ENUM_REG_ADDR_BVOLT5 = 296, // 单体电池5电压
ENUM_REG_ADDR_BVOLT6 = 297, // 单体电池6电压
ENUM_REG_ADDR_BVOLT7 = 298, // 单体电池7电压
ENUM_REG_ADDR_BVOLT8 = 299, // 单体电池8电压
ENUM_REG_ADDR_BVOLT9 = 300, // 单体电池9电压
ENUM_REG_ADDR_BVOLT10 = 301, // 单体电池10电压
ENUM_REG_ADDR_BVOLT11 = 302, // 单体电池11电压
ENUM_REG_ADDR_BVOLT12 = 303, // 单体电池12电压
ENUM_REG_ADDR_BVOLT13 = 304, // 单体电池13电压
ENUM_REG_ADDR_BVOLT14 = 305, // 单体电池14电压
ENUM_REG_ADDR_BVOLT15 = 306, // 单体电池15电压
ENUM_REG_ADDR_BVOLT16 = 307, // 单体电池16电压


ENUM_REG_ADDR_MAXCHGV = 315,//最大充电电压
ENUM_REG_ADDR_TVST = 317, // TVS温度
ENUM_REG_ADDR_BHR1 = 318, //电池1温升速度
ENUM_REG_ADDR_BHR2 = 319, //电池2温升速度
ENUM_REG_ADDR_FGHR = 320, // 电量计温升速度
ENUM_REG_ADDR_DROCV = 321, // 电芯电压最大下降速度
ENUM_REG_ADDR_IDROCV = 322, // 电芯电压最大下降速度编号


ENUM_REG_ADDR_CTRL = 512, 
ENUM_REG_ADDR_RESET = 534,


//USER ID 
ENUM_REG_ADDR_USERID12 = 4097,
ENUM_REG_ADDR_USERID34 = 4098,
ENUM_REG_ADDR_USERID56 = 4099,
ENUM_REG_ADDR_USERID78 = 4100,

};


//每次轮询的时间长度
#define BAT_POLL_ON_TIME_MAX 10000
// 轮询电池间隔时间设置
#define BAT_POLL_INTERVAL_MAX 60000
#define BAT_POLL_BIG_INTERVAL_MAX 86400000 // 24小时
#define BAT_POLL_BIG_BIG_INTERVAL_MAX 0xFFFFFFFE//259200000 // 72小时
// 仓门有变化时候，一直处于直轮询模式1分钟，过后再根据实际情况设置模式
#define BAT_POLL_DELAY_MAX 60000//1800000//360000         //改为60秒
//接收到升级BMS的start 命令，允许正常查询电池的时间，10分钟
#define BAT_POLL_BMS_UPDATE_DELAY_MAX 600000
//当有透传命令给电池时候，停止间隔轮询180秒
#define BAT_POLL_DELAY_MAX_BY_BYPASS_CMD 180000
typedef struct{
unsigned char is_status_changed_on_door;
unsigned char is_poll_allways_en_flag; // 0, 表示有时间间隔的轮询；1，表示一直轮询，无间隔时间
unsigned int poll_inerval_timer_cnt;
unsigned int poll_interval_MAX;
unsigned int poll_allways_en_delay_cnt; // 在仓门有变化的时候再reset计数，保证刚插进去的电池能轮询三分钟
unsigned int poll_interval_for_bms_update_cnt;
}st_bat_poll_param;
GLOBAL_BPROC st_bat_poll_param sl_bat_poll_param;


//电池插入时候，使用充电器来检测电池是否在位
typedef struct{
unsigned char is_bat_v_valid; //充电器是否检测到电池输出电压
//是否需要检测电池电压，当检测到有电池，电池SOC，仓门关闭就开始检测
unsigned char is_need_bat_v_checking; // 0， 不需要检测；1，正在检测；2，检测完毕
unsigned short bat_v_charger_cmd_cnt; //开始打开电池预放后，与充电器通讯的次数
unsigned int bat_v_charger_comm_timeout_cnt; //开始打开电池预放后，与充电器通讯的时间计数
unsigned short bat_v_value; //充电器检测到的电池输出电压值，单位0.1V
unsigned short bat_v_charger_check_totol_time_cnt; //整个过程的最大时间。
unsigned char bat_v_charger_charge_valid_cnt; // 充电器有电流输出并且电池有电流输入的次数，充电器应答时候计数
}st_check_bat_v;
GLOBAL_BPROC st_check_bat_v gl_check_bat_v;



typedef struct{
//标记停止充电的原因源
unsigned int src_of_stop_flag;
unsigned char bat_ID[6];
unsigned char stop_flag;
unsigned int current_diff_counter;
unsigned int bat_current_out_counter; 
unsigned int hall_miss_status_clear_delay_cnt;
//选择哪些原因源可以被设置到src_of_stop_flag中
unsigned int src_of_stop_flag_disable_flag; // 对应src_of_stop_flag中的每个Bit，0是允许，1是禁止
}st_event_to_stop_chg_param;
GLOBAL_BPROC st_event_to_stop_chg_param gl_event_to_stop_chg_param;

typedef struct{
    unsigned char is_bat_chg_enable; // 0, 不允许充电，1，允许前期小电流充电，2，允许正常充电
    unsigned char is_need_to_reflash_temp; //是否需要刷新电池插入时候的电池温度值
    short temp_value;//插入电池时候电池最低温度，单位是0.1℃
    unsigned int delay_counter_1;//第一阶段计数器
    unsigned int delay_counter_2;//第二阶段计数器
    unsigned int delay_counter_charge; //前期充电阶段计数
    unsigned int low_current_delay_count; //低电流充电的时间长度
}st_bat_low_temp_param;
GLOBAL_BPROC st_bat_low_temp_param gl_bat_low_temp_param;

typedef struct{
    short low_temp; // 0.1℃为单位
    unsigned int low_temp_delay_cnt; // 1mS为单位
    short high_temp;// 0.1℃为单位
    unsigned int high_temp_delay_cnt;// 1mS为单位
    unsigned int low_current_delay_count;// 1mS为单位
}st_bat_low_temp_ctrl_param;

//当充电器输出电压精度有问题导致输出电压偏低时候，适当调高充电器输出电压。
typedef struct{
unsigned char reseve[3];
unsigned char voltage_offset_value; // 单位0.1V
unsigned int debount_time_cnt;
//当前期望充电电压(未加偏移量)，单位0.01V
unsigned short expect_setting_voltage;
//当前期望充电电流(未加偏移量)，单位0.01A
unsigned short expect_setting_current;
}st_charger_setting_V_offset_param;
GLOBAL_BPROC st_charger_setting_V_offset_param gl_charger_setting_V_offset_param;

//防盗流程的参数
typedef struct{
unsigned char connector_default_state:1; //连接器状态; 1, 未连接; 0, 连接好
unsigned char switch_default_state:1; //微动开关状态; 1, 未到位; 0, 到位
unsigned char reserve_bit:6;
}st_bat_theft_prevention_param;
#ifndef BATTERY_PROCESS_C
GLOBAL_BPROC st_bat_theft_prevention_param gl_bat_theft_prevention_param;
#else
GLOBAL_BPROC st_bat_theft_prevention_param gl_bat_theft_prevention_param = {0, 0, 0};
#endif


/*
读取电池历史记录
*/
//历史记录编号的范围
#define HISTORY_RECORD_INDEX_START 1
#define HISTORY_RECORD_INDEX_END 536862720

typedef struct{
//发送读取历史数据的NFC通讯命令的间隔时间
unsigned int NFC_CMD_interval_cnt;
//需要读取的当前记录编号，真正需要读取的是从下一个编号开始
unsigned int history_record_begin_index;
//需要读取的记录数目
unsigned int history_record_cnt;
//当前已经读取的记录数目
unsigned int history_record_read_cnt;
//当前电池最早记录编号
unsigned int history_record_earliest_index;
//当前电池最近记录编号
unsigned int history_record_lastest_index;
//需要读取的当前记录编号的时间 。 BCD码
unsigned char history_record_begin_index_datetime[6];
//当前电池最早记录编号的时间
unsigned char history_record_earliest_index_datetime[6];
//当前电池最近记录编号的时间
unsigned char history_record_lastest_index_datetime[6];
//缓存历史数据
unsigned char history_record_buf[128];
//电池ID
unsigned char bat_ID[6];
//起始记录编号和数目是否有效
unsigned char is_index_valid:1;
//buffer中的数据是否有效
unsigned char is_buf_valid:1;
//buffer中的数据是否已经上传
unsigned char is_buf_updated:1;
//当前电池最早记录编号和时间的值是否有效
unsigned char is_earliest_param_valid:1;
//当前电池最近记录编号和时间的值是否有效
unsigned char is_lastest_param_valid:1;
unsigned char bit_reserve:4;
}st_bat_history_record_param;

GLOBAL_BPROC st_bat_history_record_param gl_bat_history_record_param;



//BAT_ERROR_CHECK_DELAY_AFTER_BAT_UPDATE
GLOBAL_BPROC unsigned int sl_bat_error_check_delay_after_bat_update_cnt;


typedef struct{
    unsigned char OD_update_file_max_subindex;
    /*升级文件的硬件版本号*/
    unsigned char OD_update_file_HW_version[2];
    /*升级文件的固件版本号*/
    unsigned char OD_update_file_FW_version[7];
    /*升级文件的buffer地址*/
    unsigned int OD_update_file_buff_addr;
}st_update_file_param;

GLOBAL_BPROC void Battery_clear_reg_valid_flag(unsigned char bms_index);

GLOBAL_BPROC void Battery_Process(void);
GLOBAL_BPROC unsigned short Battery_get_reg_value(unsigned char bms_index, unsigned short reg_index);
GLOBAL_BPROC unsigned short Battery_get_voltage(unsigned char bms_index);
GLOBAL_BPROC unsigned short Battery_get_SOC(unsigned char bms_index);


GLOBAL_BPROC int Battery_get_current(unsigned char bms_index);
GLOBAL_BPROC unsigned char is_battery_error(unsigned char bms_index);
GLOBAL_BPROC bool is_battery_A_V_reg_valid(unsigned char bms_index);
GLOBAL_BPROC void Battery_cmd_buf_clear(unsigned char bms_index);
GLOBAL_BPROC bool Battery_is_one_cmd_buf_empty(unsigned char bms_index);

GLOBAL_BPROC unsigned char Battery_get_reg_valid_flag(unsigned char bms_index);
GLOBAL_BPROC unsigned char Battery_send_cmd(unsigned char bms_index);

GLOBAL_BPROC unsigned short Battery_get_switch_state(unsigned char bms_index);
GLOBAL_BPROC unsigned char slave_rs485_is_bat_valid(unsigned char bms_index);
GLOBAL_BPROC void thread_battery_entry(void* parameter);
GLOBAL_BPROC void Battery_timer_count_cb(void);

#define BAT_ERR_SIZE 		6
#define SERIAL_NUM_SIZE		6


#if 1
typedef struct _Battery
{
	uint8  port; 			//
	
	uint8  isPresent:1; 	//is in port
	uint8  isCharged:1; 		
	uint8  Reserved:6; 		

	//Battery properties
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint8  soc;
	uint8  soh;
	uint8  temperature;
	uint16 voltage;
	uint16 current;
	uint16 cycleCount; 	//
	uint16 capacity; 	//
	
	uint8  batteryErr[BAT_ERR_SIZE]; //故障码
}Battery;
#endif

void Battery_Init(Battery* pBattery, uint8 nPort);


#endif // MID_CONTROL_PROTOCOL_H__

/** @} */

