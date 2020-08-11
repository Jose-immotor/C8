

/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef MODBUS_MASTER_H__
#define MODBUS_MASTER_H__

#include "fm175xx.h"

#ifndef MODBUS_MASTER_C
#define GLOBAL_MM extern
#else
#define GLOBAL_MM
#endif

//modbus状态机
typedef enum{
    ENUM_MM_NFC_INIT,//NFC处于初始化状态
    ENUM_MM_IDLE,//空闲状态
    ENUM_MM_BC_WAIT,//等待回转延迟
    ENUM_MM_ACK_WAIT,//等待应答
    ENUM_MM_ACK_PROCESS,//处理应答
    ENUM_MM_ERR_PROCESS//处理差错
}ENUM_MODBUS_MASTER_STATE;

//接收数据包处理状态机
typedef enum{
    ENUM_MD_INIT,//初始状态
    ENUM_MD_IDLE,//空闲状态
    ENUM_MD_SND,//发送状态
    ENUM_MD_RCV,//接收状态
    ENUM_MD_CTRL//控制和等待状态
}ENUM_MODBUS_DATA_RCV_STATE;

#define MM_FN_CODE_READ_REG 0x03
#define MM_FN_CODE_WRITE_REG 0x06
#define MM_FN_CODE_WRITE_MULTI_REG 0x10
#define MM_FN_CODE_READ_HR_REG 0x46 // Histroy Record 历史数据
#define MM_FN_CODE_UPDATE_REG 0x67
#define MM_FN_CODE_AUTHORITY_REG 0x68
#define MM_FN_CODE_BYPASS 0x7F //自定义，用于自己识别是透传数据


//缓存最大长度
#define MM_SND_BUFF_MAX 138
#define MM_RCV_BUFF_MAX 138

//包头信息
#define MM_PCB_VALUE 0x02
#define MM_RESERVE_VALUE 0x00
#define MM_ADDR_VALUE 0x01

//数据包最大重发次数
#define MM_NFC_SND_ERR_MAX 5

//超时时间
#define MM_RSP_WAIT_TIMER_CNT_MAX 1500
#define MM_BC_WAIT_TIMER_CNT_MAX 200

// 1字节对齐
//#pragma pack(1)
typedef struct{
//    ENUM_MODBUS_MASTER_STATE mm_state[NFC_READER_COUNT_MAX];
    //ENUM_MODBUS_DATA_RCV_STATE md_rcv_state[NFC_READER_COUNT_MAX];
    //unsigned char need2snd_data;//需要发送数据包
//    unsigned char BMS_index; //目前通讯的电池序号
    unsigned char slave_addr[NFC_READER_COUNT_MAX];//发送时候的从机地址，0 表示广播, 从机有效地址1~247
    unsigned char MM_fn_code[NFC_READER_COUNT_MAX];//请求的功能码
    unsigned char MM_NFC_snd_buff[NFC_READER_COUNT_MAX][MM_SND_BUFF_MAX];//发送缓存
    unsigned char MM_NFC_rcv_buff[NFC_READER_COUNT_MAX][MM_RCV_BUFF_MAX];//接收缓存
    unsigned char MM_NFC_snd_err_cnt[NFC_READER_COUNT_MAX];//发送错误计数
    unsigned char MM_NFC_nead_reset[NFC_READER_COUNT_MAX]; //等于1表示NFC读卡器需要重启
    unsigned int MM_NFC_snd_len[NFC_READER_COUNT_MAX];//发送缓存中存在的数据长度
    unsigned int MM_NFC_rcv_len[NFC_READER_COUNT_MAX];//接收缓存中存在的数据长度
//    unsigned int MM_IDLE_timer_cnt;
    //unsigned char MM_NFC_snd_enable;
}st_modbus_param;



#pragma pack(1)
typedef struct{

    unsigned char reg_addr_begin[2]; //大端，起始地址
    unsigned char reg_cnt[2];  //大端，寄存器数量

}st_read_reg_03;

typedef struct{

    unsigned char reg_addr[2]; //大端，寄存器地址
    unsigned char reg_value[2]; //大端，寄存器值

}st_write_reg_06;

typedef struct{

    unsigned char reg_addr_begin[2]; //大端，起始地址
    unsigned char reg_cnt[2];  //大端，寄存器数量
    unsigned char byte_cnt;  //大端，字节计数
    unsigned char *byte_data; //寄存器值
}st_write_reg_10;
#pragma pack()






//对齐结束
//#pragma pack()

GLOBAL_MM st_modbus_param gl_modbus_param;

GLOBAL_MM unsigned char MM_check_packet_head(unsigned char *pl_buff, unsigned char slave_addr,unsigned char fn_code);

GLOBAL_MM void MM_init(unsigned char bms_index);
GLOBAL_MM unsigned char MM_is_snd_done(void);
GLOBAL_MM unsigned char MM_is_snd_done_by_index(unsigned char bms_index);
GLOBAL_MM void MM_NFC_init_done(unsigned char bms_index);
GLOBAL_MM void MM_NFC_snd_done_cb(void);
GLOBAL_MM unsigned char MM_NFC_snd_ERR(void);
GLOBAL_MM void MM_timer_count_cb(void);
GLOBAL_MM unsigned char MM_set_BMS_index(unsigned char pl_index);
GLOBAL_MM unsigned char MM_is_bypass_fn_code_working(void);
GLOBAL_MM unsigned char MM_is_read_HR_fn_code_working(void);

GLOBAL_MM void MM_task(void);

GLOBAL_MM unsigned char MM_snd_multi_read_cmd(unsigned char slave_addr,st_read_reg_03* pl_read_reg_03);
GLOBAL_MM unsigned char MM_snd_multi_write_cmd(unsigned char slave_addr,st_write_reg_10* pl_write_reg_10);
GLOBAL_MM unsigned char MM_snd_read_history_record_cmd(unsigned char slave_addr, unsigned int record_index);
GLOBAL_MM unsigned char MM_snd_bypass(unsigned char slave_addr,unsigned char* data, unsigned int data_len);

GLOBAL_MM unsigned char MM_is_all_snd_done(void);

#endif // 
/** @} */
