/*****************************************************************
*Company: BSJ
*Creator:
*Project: 
*Module name: 
*
*########################################
*Compiler: 
    armcc --vsn: RVCT3.1 Build569
    armar --vsn: RVCT3.1 Build1021
    armasm --vsn: RVCT3.1 Build569
    armlink --vsn: RVCT3.1 Build569
    fromelf --vsn: RVCT3.1 Build569
    tcc --vsn: RVCT3.1 Build569

*MCU:  MT2503D
*CLOCK: 260Mhz ROM:4MB RAM:4MB
*########################################
*
*File Name: 
*Purpose: 
*File References: 
*Create Date: Jul-06-2016
*Change History
*Description
******************************************************************/

#ifndef _GSENSOR_H__
#define _GSENSOR_H__

#include "config.h"
#include "global_types_def.h"
#include "typedef.h"

#define SUPPORT_DA213
#define SUPPORT_3413

// Export defines
#define GSENSOR_SAVE_FLG     0XA5
#define GSENSOR_ID_ERROR_CNT 400  // 10S   20ms for one



// Export typedefs
#ifndef KM08_ST   //此参数，统一放在typedef.h中
#define GSENSOR_BUFF_LEN            50+1
#define GSENSOR_ABNORMAL_TIM 45000

typedef struct
{
    
    u8  sta;            // 工作状态,正常，异常
    u16 sonser_zdvlaue; //
    u16 max_dif;        // 最大XYZ矢量和差值
    u16 minor_max_dif;  // 次最大XYZ矢量和差值
    #ifdef SUPPORT_3413
    u8 PCode;
    #endif
    #ifdef GSENSOR_BUG_20170509
    u16 staticValue;
    #else
    u8  vec_len;        // 矢量和缓冲区计数
    u16 vec_buff[GSENSOR_BUFF_LEN];
    #endif

    u8  checkTim;
} gsensor_type;
extern  gsensor_type g_gsensor;
#endif

// MC3430异常处理 12
typedef struct
{
    u8  flg;            // 标志
    u8  abn_flg;        // 异常标志
    u16 id_err_cnt;     // 读取ID号异常次数
    u16 xyz_err_cnt;    // 读取XYZ轴异常次数
    u16 tx_id_err_cnt;  // 发送读取ID号异常次数
    u16 tx_xyz_err_cnt; // 发送读取XYZ轴异常次数
    u8 xyz_buff[3];     // XYZ轴值
    u16 xyz_err_cnt_ff;    // 读取XYZ轴异常为0xff次数
} gsensor_abn_t;
extern  gsensor_abn_t  gsensor_abn;

// Export enums
typedef enum
{
    QL_RET_OK = 0
} ql_ret_e;

typedef enum
{
    MC3430_D_STATUS  = 0x00,      //data status

    MC3430_X_MSB  = 0x01,     //OUT_X_MSB(1)
    MC3430_X_LSB  = 0x02,     //OUT_X_LSB(1)
    MC3430_Y_MSB  = 0x03,     //OUT_Y_MSB(1)
    MC3430_Y_LSB  = 0x04,     //OUT_Y_LSB(1)
    MC3430_Z_MSB  = 0x05,     //OUT_Z_MSB(1)
    MC3430_Z_LSB  = 0x06,     //OUT_Z_LSB(1)

    MMA_RO_STATUS           = 0x00,     //STATUS  MMA8653R_D_STATUS
    MMA_RO_X_MSB            = 0x01,  //OUT_X_MSB(1)
    MMA_RO_X_LSB            = 0x02,  //OUT_X_LSB(1)
    MMA_RO_Y_MSB            = 0x03,  //OUT_Y_MSB(1)
    MMA_RO_Y_LSB            = 0x04,  //OUT_Y_LSB(1)
    MMA_RO_Z_MSB            = 0x05,  //OUT_Z_MSB(1)
    MMA_RO_Z_LSB            = 0x06,  //OUT_Z_LSB(1)
    MMA_RO_SYSMOD           = 0x0B,     //SYSMOD
    MMA_RO_INTSRC           = 0x0C,     //INT SOURCE
    MMA_RO_WHOAMI           = 0x3B,    //Device ID (0x39), MMA_REG_DEVICE_ID
    MMA_RW_XZY_DATA_CFG     = 0x0E,     //XYZ_DATA_CFG
    MMA_RO_PL_STATUS        = 0x10,     //PL_STATUS
    MMA_RW_PL_CFG           = 0x11,
    MMA_RW_PL_COUNT         = 0x12,
    MMA_RO_BF_ZCOMP         = 0x13,
    MMA_RO_THS_REG          = 0x14,
    MMA_RW_FF_MT_CFG        = 0x15,
    MMA_RO_FF_MT_SRC        = 0x16,
    MMA_RW_FF_MT_THS        = 0x17,
    MMA_RW_FF_MT_COUNT      = 0x18,
    MMA_RW_ALSP_COUNT       = 0x29,
    MMA_RW_CTRL_REG1        = 0x2A,  //Data Rates, ACTIVE Mode.
    MMA_RW_CTRL_REG2        = 0x2B,  //Sleep Enable, OS Modes, RST, ST
    MMA_RW_CTRL_REG3        = 0x2C,  //Wake from Sleep, IPOL, PP_OD
    MMA_RW_CTRL_REG4        = 0x2D,  //Interrupt enable register
    MMA_RW_CTRL_REG5        = 0x2E,  //Interrupt pin (INT1/INT2) map
    MMA_RW_OFF_X            = 0x2F,     //X Offset
    MMA_RW_OFF_Y            = 0x30,     //Y Offset
    MMA_RW_OFF_Z            = 0x31,     //Z Offset
    MMA_RW_NULL
} mma_reg_e;

#ifdef SUPPORT_DA213
#define NSA_REG_SPI_I2C                 0x00
#define NSA_REG_WHO_AM_I                0x01
#define NSA_REG_ACC_X_LSB               0x02
#define NSA_REG_ACC_X_MSB               0x03
#define NSA_REG_ACC_Y_LSB               0x04
#define NSA_REG_ACC_Y_MSB               0x05
#define NSA_REG_ACC_Z_LSB               0x06
#define NSA_REG_ACC_Z_MSB               0x07
#define NSA_REG_MOTION_FLAG				0x09
#define NSA_REG_G_RANGE                 0x0f
#define NSA_REG_ODR_AXIS_DISABLE        0x10
#define NSA_REG_POWERMODE_BW            0x11
#define NSA_REG_SWAP_POLARITY           0x12
#define NSA_REG_FIFO_CTRL               0x14
#define NSA_REG_INTERRUPT_SETTINGS1     0x16
#define NSA_REG_INTERRUPT_SETTINGS2     0x17
#define NSA_REG_INTERRUPT_MAPPING1      0x19
#define NSA_REG_INTERRUPT_MAPPING2      0x1a
#define NSA_REG_INTERRUPT_MAPPING3      0x1b
#define NSA_REG_INT_PIN_CONFIG          0x20
#define NSA_REG_INT_LATCH               0x21
#define NSA_REG_ACTIVE_DURATION         0x27
#define NSA_REG_ACTIVE_THRESHOLD        0x28
#define NSA_REG_TAP_DURATION            0x2A
#define NSA_REG_TAP_THRESHOLD           0x2B
#define NSA_REG_CUSTOM_OFFSET_X         0x38
#define NSA_REG_CUSTOM_OFFSET_Y         0x39
#define NSA_REG_CUSTOM_OFFSET_Z         0x3a
#define NSA_REG_ENGINEERING_MODE        0x7f
#define NSA_REG_SENSITIVITY_TRIM_X      0x80
#define NSA_REG_SENSITIVITY_TRIM_Y      0x81
#define NSA_REG_SENSITIVITY_TRIM_Z      0x82
#define NSA_REG_COARSE_OFFSET_TRIM_X    0x83
#define NSA_REG_COARSE_OFFSET_TRIM_Y    0x84
#define NSA_REG_COARSE_OFFSET_TRIM_Z    0x85
#define NSA_REG_FINE_OFFSET_TRIM_X      0x86
#define NSA_REG_FINE_OFFSET_TRIM_Y      0x87
#define NSA_REG_FINE_OFFSET_TRIM_Z      0x88
#define NSA_REG_SENS_COMP               0x8c
#define NSA_REG_SENS_COARSE_TRIM        0xd1
#endif
// Export macros
// Export variables
#ifdef IMPACT_ALARM_ENABLE
#define IMPACT_ALARM   0x01

#define imST_IDLE   	0x00        /// 空闲阶段
#define imST_ACCE  	0x01		///  Acce检测
#define imST_DECT  	0x02        /// 检测阶段
#define imST_LAST   	0x03        /// 持续阶段

typedef struct
{
    u8  cImpaAla;
    u8  cImpaSmsAla;
    u32 imMmaValue;

    u8   imState;
    u16  imTimer;
    u16  imFlag;
    u8  impact_evt;   // 碰撞事件
    u32 impact_det_tim;
}ImpackAlaType;

extern ImpackAlaType g_stuImpAl;

void InitImpackAlarm(void);
void MmaImpact_Pro(void);
void imNextState(u8);
#endif

extern const u16 ShakeAlmValArr[];

// Export functions
extern void gsensor_Init(void);
extern void gsensor_prj(void);


void gsensor_getAccelerate(void);

u8 gsensor_getDeviceCode(void);

u16 G_sensor_xyz(t_gsensor_type *gsensor);

void gsensor_power_on(void);
void gsensor_power_down(void);
u8 gsensor_shaking_alarm(void);
u8 JudgeTermiWorkSta(u8 x,u8 y,u8 z);
void GetGsensorVal(u8 xyz,u32 *pAcceVlu,u16 *signVal);
void Ffmt_Zthus_Init(void);
void Ffmt_Zthus_Pro(void);

#ifdef IMPACT_ALARM_ENABLE
void CheckImpack(void);
#endif

#endif

