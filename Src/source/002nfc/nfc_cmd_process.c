

#define NFC_CMD_PROCESS_C
#include <string.h>
#include <stdio.h>
//#include "HMAC.h"
#include "modbus_master.h"
////#include "xmodem.h"
#include "ntag_app.h"
#include "nfc_cmd_process.h"
#include "battery.h"
////#include "rs485_protocol.h"

////extern unsigned char vl_BMS_index; //从0开始

const st_nfc_cmd_table gl_nfc_cmd_table[] = {
    //读保持寄存器
    {MM_FN_CODE_READ_REG, MM_read_reg_rsp_cb, MM_read_reg_err_cb},
    //写单个寄存器
    {MM_FN_CODE_WRITE_REG, MM_write_reg_rsp_cb, MM_write_reg_err_cb},
    //写多个寄存器
    {MM_FN_CODE_WRITE_MULTI_REG, MM_write_multi_reg_rsp_cb, MM_write_multi_reg_err_cb},
    //升级BMS功能码
//    {MM_FN_CODE_UPDATE_REG, MM_snd_update_packet_cb, MM_snd_update_packet_err_cb},
};

void MM_nfc_cmd_init(void)
{   
    unsigned char i;
    for(i = 0; i < NFC_READER_COUNT_MAX; i++)
    {
        gl_bms_info_p[i]= NULL;
    }
	#if (NFC_READER_COUNT_MAX > 0)
        memset(&gl_bms_info_0, 0, sizeof(st_bms_info));
        gl_bms_info_p[0]= &gl_bms_info_0;
    #endif

    #if (NFC_READER_COUNT_MAX > 1)
        if(1 < NFC_READER_COUNT_MAX)
        {
            memset(&gl_bms_info_1, 0, sizeof(st_bms_info));
            gl_bms_info_p[1]= &gl_bms_info_1;
        }
    #endif
    
    #if (NFC_READER_COUNT_MAX > 2)
    if(2 < NFC_READER_COUNT_MAX)
    {
        memset(&gl_bms_info_2, 0, sizeof(st_bms_info));
        gl_bms_info_p[2]= &gl_bms_info_2;
    }
    #endif

    #if (NFC_READER_COUNT_MAX > 3)
    if(3 < NFC_READER_COUNT_MAX)
    {
        memset(&gl_bms_info_3, 0, sizeof(st_bms_info));
        gl_bms_info_p[3]= &gl_bms_info_3;
    }
    #endif

    #if (NFC_READER_COUNT_MAX > 4)
    if(4 < NFC_READER_COUNT_MAX)
    {
        memset(&gl_bms_info_4, 0, sizeof(st_bms_info));
        gl_bms_info_p[4]= &gl_bms_info_4;
    }
    #endif

    #if (NFC_READER_COUNT_MAX > 5)
    if(5 < NFC_READER_COUNT_MAX)
    {
        memset(&gl_bms_info_5, 0, sizeof(st_bms_info));
        gl_bms_info_p[5]= &gl_bms_info_5;
    }
    #endif

    #if (NFC_READER_COUNT_MAX > 6)
    Haved to add more st_bms_info struct, ex. gl_bms_info_5
    #endif

    memset(gl_bms_app_bl_status, 0, NFC_READER_COUNT_MAX * sizeof(st_bms_app_bl_status));
}

/*
响应命令处理
return : 0, 错误；1,正确
*/
unsigned char MM_rsp_cmd_process(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    unsigned char i, vl_max, vl_fn;
    vl_max = sizeof(gl_nfc_cmd_table)/sizeof(st_nfc_cmd_table);
    vl_fn = pl_rcv_buff[4]&0x7F;
    for(i = 0; i < vl_max; i++)
    {
        if(vl_fn == gl_nfc_cmd_table[i].fn_code)
        {
            if(gl_nfc_cmd_table[i].function_rsp_cb != NULL)
                return gl_nfc_cmd_table[i].function_rsp_cb(pl_rcv_buff,
                                                            rcv_buf_len,
                                                            pl_snd_buff,
                                                            snd_buf_len);
            //退出查询处理
            break;
        }
    }
    return 1;
}

unsigned char MM_err_process(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    //帧差错，响应超时
    unsigned char i, vl_max, vl_fn;
    vl_max = sizeof(gl_nfc_cmd_table)/sizeof(st_nfc_cmd_table);
    vl_fn = pl_rcv_buff[4]&0x7F;
    for(i = 0; i < vl_max; i++)
    {
        if(vl_fn == gl_nfc_cmd_table[i].fn_code)
        {
            if(gl_nfc_cmd_table[i].function_snd_faild_cb != NULL)
                return gl_nfc_cmd_table[i].function_snd_faild_cb(pl_rcv_buff,
                                                            rcv_buf_len,
                                                            pl_snd_buff,
                                                            snd_buf_len);
            //退出查询处理
            break;
        }
    }
    return 1;
}

//获取寄存器地址在存储区中的位置
unsigned short MM_get_reg_addr_position_in_table(unsigned short pl_reg_addr)
{
    unsigned short i;
    /*
    0~24
    256~308
    512~533
    768~830
    4096~4100
    */

    if(pl_reg_addr <= DEV_INFO_REG_END_ADDR)
    {
        return pl_reg_addr;
    }
    i = (DEV_INFO_REG_TOTAL_SIZE);
    if((READ_ONLY_REG_BIGIN_ADDR <= pl_reg_addr)&&(pl_reg_addr <= READ_ONLY_REG_END_ADDR))
    {
        return (i+(pl_reg_addr-READ_ONLY_REG_BIGIN_ADDR));
    }
    i += (READ_ONLY_REG_TOTAL_SIZE);
    if((CTRL_REG_BIGIN_ADDR <= pl_reg_addr)&&(pl_reg_addr <= CTRL_REG_END_ADDR))
    {
        return (i+(pl_reg_addr-CTRL_REG_BIGIN_ADDR));
    }
    i += (CTRL_REG_TOTAL_SIZE);

    if((PARAM_REG_BIGIN_ADDR <= pl_reg_addr)&&(pl_reg_addr <= PARAM_REG_END_ADDR))
    {
        return (i+(pl_reg_addr-PARAM_REG_BIGIN_ADDR));
    }
    i += (PARAM_REG_TOTAL_SIZE);

    if((USER_REG_BIGIN_ADDR <= pl_reg_addr)&&(pl_reg_addr <= USER_REG_END_ADDR))
    {
        return (i+(pl_reg_addr-USER_REG_BIGIN_ADDR));
    }

    //因为没有机制保证所找的寄存器地址一定在存储范围内，
    //所以返回最后一个用户数据的位置
    return BMS_MODBUS_REG_MAX-1;
    
}

/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_read_reg_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    unsigned short vl_reg_begin_addr;
    unsigned short i;

    if(pl_rcv_buff[4]&0x80)
    {
        //从机返回出现错误的相应
        
        return 1;
    }

    //如果不是2的倍数，返回错误
    if(pl_rcv_buff[5]%2)
    {
        return 0;
    }

    //判断发送的数据是否已经被清空
    if(snd_buf_len < 7)
    {
        return 0;
    }
    //寄存器的开始地址
    vl_reg_begin_addr = pl_snd_buff[5]<<8;
    vl_reg_begin_addr += pl_snd_buff[6];

    //如果是正在设置ctrl寄存器，则不再存数据
    if((vl_reg_begin_addr == CTRL_REG_BIGIN_ADDR)&&
        (gl_cmd_buf[gl_NTAGTX_param.vl_BMS_index][ENUM_NFC_CMD_INDEX_W_CTRL]))
    {
        return 1;
    }
    
    i = MM_get_reg_addr_position_in_table(vl_reg_begin_addr);
    if(i == BMS_MODBUS_REG_MAX) //越界，返回错误
        return 0;
    memcpy(&(gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->reg_unit[i]), &pl_rcv_buff[6], pl_rcv_buff[5]);  
    
    return 1;
}
/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_read_reg_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    //帧差错，响应超时
    //重新初始化NFC任务
    //NTAG_task_reset();
    
    return 1;
}
/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_write_reg_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //从机返回出现错误的相应
        return 1;
    }

    if(0 != memcmp(pl_rcv_buff, pl_snd_buff,snd_buf_len))
    {
        //设置有错
        return 0; 
    }
    return 1;
}
/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_write_reg_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    return 1;
}
/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_write_multi_reg_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //从机返回出现错误的相应
        return 1;
    }

    if(0 != memcmp(pl_rcv_buff, pl_snd_buff,9))
    {
        //设置有错
        return 0; 
    }
    return 1;
}
/*
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_write_multi_reg_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    return 1;
}

static unsigned char char_to_hex(unsigned char str)
{
    if(('0' <= str)&&(str <= '9'))
    {
        return (str-'0');
    }
    else if(('a' <= str)&&(str <= 'f'))
    {
        return (str-'a'+10);
    }
    else if(('A' <= str)&&(str <= 'F'))
    {
        return (str-'A'+10);
    }


    return 0;
}

///*
//NFC应答升级包
//return: 0, 处理错误; 1, 处理正确
//*/
//st_update_buf gl_update_buf;
//unsigned char MM_snd_update_packet_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    if(pl_rcv_buff[4]&0x80)
//    {
//        //从机返回出现错误的相应

//        #ifdef BAT_ERROR_CHECK_DELAY_AFTER_BAT_UPDATE
//        if(gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode)
//        {
//            sl_bat_error_check_delay_after_bat_update_cnt = 120000; // 120秒
//            //清除主动测试电池故障的流程
//            Battery_check_bat_clear_on_and_error();
//        }
//        #endif

//        //标识处于APP状态
//        gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 0;
//        return 1;
//    }

//    //标识处于bootloader状态
//    gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 1;
//    
//    //如果不在升级状态
//    if((gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_STATE_CHECK_IDLE)&&
//        (gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_IDLE))
//    {
//        unsigned char i;
//        
//        for(i = 5; i < rcv_buf_len; i++)
//        {
//            //HW Ver:
//            if((gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i] == 'H')&&
//                (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+1] == 'W')&&
//                (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+2] == ' ')&&
//                (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+3] == 'V')&&
//                (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+4] == 'e')&&
//                (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+5] == 'r'))
//            {
//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_main_version = char_to_hex(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+8]);
//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_main_version <<= 4;
//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_main_version += char_to_hex(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+9]);

//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_sub_version = char_to_hex(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+11]);
//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_sub_version <<= 4;
//                gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].hw_sub_version += char_to_hex(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][i+12]);                    
//                
//                break;
//            }
//        }
//    }

//    if((gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_CHECK_PROCESS)&&
//        (gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_STATE_CHECK_RCV))
//    {
//        //探测是否是bootloader模式
//        //正确返回
//        gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_XMODEM_PROCESS;
//        gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//        gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_START;
//    }

//    if((gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_XMODEM_PROCESS)&&
//        (gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_XMODEM_START_WAIT_RCV))
//    {
//        switch (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][5])
//        {
//            case 'C':
//                //crc = 1;
//                gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_TRANS_DATA;
//                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_START, ENUM_COMM_ERROR_CODE_OK);
//                break;
//            case NAK:
//                //crc = 0;
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_TRANS_DATA;
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_NOTREADY);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_START, ENUM_COMM_ERROR_CODE_NOTREADY);
//                break;
//            case CAN:
//                //flushinput();
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;

//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_FAILED);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_START, ENUM_COMM_ERROR_CODE_FAILED);
//                break;
//            default:
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_START;

//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_UNKNOWN);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_START, ENUM_COMM_ERROR_CODE_UNKNOWN);
//                break;
//        }
//    }

//    if((gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_XMODEM_PROCESS)&&
//        (gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_XMODEM_TRANS_DATA_WAIT_RCV))
//    {
//        //等待返回
//        switch (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][5])
//        {
//            case ACK:
//                //bms返回正确应答
//                //++packetno;
//                //len += bufsz;
//                //goto start_trans;

//                gl_update_buf.data_valid_flag = 0;
//                gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_TRANS_DATA;

//                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_OK);
//                break;
//            case CAN:
//                //bms拒绝，停止升级
//                //flushinput();
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_FAILED);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_FAILED);
//                break;
//            case NAK:
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_FAILED);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_FAILED);
//                break;
//            default:
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_UNKNOWN);
//                break;
//        }
//    }

//    if((gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_XMODEM_PROCESS)&&
//        (gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_XMODEM_END_WAIT_RCV))
//    {
//        switch (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][5])
//        {
//            case ACK:
//                #if 0
//                gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_OK);
//                #else
//                gl_update_buf.data_valid_flag = 0;
//                gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_REBOOT;
//                #endif
//                break;
//            case CAN:
//                //bms拒绝，停止升级
//                //flushinput();
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_FAILED);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_FAILED);
//                break;
//            case NAK:
//                //gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//                //gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//                //gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_FAILED);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_FAILED);
//                break;
//            default:
//                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_UNKNOWN);
////                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_UNKNOWN);
//                break;
//        }
//    }

//    if((gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_BMS_UPDATE_XMODEM_PROCESS)&&
//        (gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_XMODEM_REBOOT_WAIT_RCV))
//    {
//        //Boot OK
//        if((gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][7] == 'B')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][8] == 'o')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][9] == 'o')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][10] == 't')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][11] == ' ')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][12] == 'O')&&
//            (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][13] == 'K'))
//        {
//            gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_IDLE;
//            gl_bms_state_check[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_STATE_CHECK_IDLE;
//            gl_bms_update_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_BMS_UPDATE_IDLE;
//            slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_OK);
//            
//            #ifdef BAT_ERROR_CHECK_DELAY_AFTER_BAT_UPDATE
//            if(gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode)
//            {
//                sl_bat_error_check_delay_after_bat_update_cnt = 120000; // 120秒
//                //清除主动测试电池故障的流程
//                Battery_check_bat_clear_on_and_error();
//            }
//            #endif
//            //标识退出bootloader状态
//            gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 0;

//            //清除命令缓存
////            Battery_cmd_buf_clear(vl_BMS_index);
////            gl_bms_info_p[vl_BMS_index]->online = 0;
////            //清除寄存器有效的标志
////            //NTAG_clear_reg_valid_flag(vl_BMS_index);
////            Battery_clear_reg_valid_flag_on_version_info(vl_BMS_index);
//            NTAG_task_reset();
//        }
//        else
//        {
////            slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_FAILED);
//        }
//    }

//    return 1;
//}

///*
//NFC应答升级包
//return: 0, 处理错误; 1, 处理正确
//*/
//unsigned char MM_snd_update_packet_err_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    if(pl_rcv_buff[4]&0x80)
//    {
//        //从机返回出现错误的相应
//        return 1;
//    }

//    return 1;
//}


////发送数据包到BMS
//unsigned char MM_snd_update_packet(unsigned char bms_index, unsigned char slave_addr,unsigned char *data, unsigned int data_len)
//{
//    if(0 == MM_set_BMS_index(bms_index))
//    {
//        return 0;
//    }

//    //if(!MM_is_snd_done())
////    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
////    {
////        return 0;
////    }
//    
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_FN_CODE_UPDATE_REG;
//    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], data, data_len);
//    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] += data_len;

//    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
//    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_UPDATE_REG;

//    //清空接收缓存
//    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
//    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
//    //gl_modbus_param.MM_NFC_snd_len
//    //gl_modbus_param.MM_NFC_snd_buff
//    return 1;
//}



/*
NFC应答认证包
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_history_record_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //从机返回出现错误的相应        
        return 1;
    }

//    Battery_history_record_set_data(&pl_rcv_buff[5]);
    
    return 1;
}


/*
NFC应答升级包
return: 0, 处理错误; 1, 处理正确
*/
unsigned char MM_history_record_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //从机返回出现错误的相应
        return 1;
    }

    return 1;
}
