/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef __BTTTERY_H__
#define __BTTTERY_H__

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

#define SERIAL_NUM_SIZE		6

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



#endif // __BTTTERY_H__

/** @} */

