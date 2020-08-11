

#define XMODEM_C

#include <string.h>

#include "fm175xx.h"
//#include "rs485_protocol.h"
#include "xmodem_crc16.h"
#include "xmodem.h"
#include "modbus_master.h"
#include "battery.h"
#include "ntag_app.h"
#include "rs485_protocol.h"
//XMODEM命令超时时间设置
#define XMODEM_TIMEOUT_MAX 3000

void xmodem_transmit(unsigned char bms_index)
{
    switch(gl_xmodem_state[bms_index])
    {
        case ENUM_XMODEM_IDLE:
            
            break;
        case ENUM_XMODEM_START:
            {
                unsigned char vl_tmp;
                //发送'U'给BMS
                vl_tmp = 'U';
                if(MM_snd_update_packet(bms_index, MM_ADDR_VALUE, &vl_tmp, 1))
                {
                    gl_bms_reboot_wait_cnt = 1500;
                    gl_xmodem_state[bms_index] = ENUM_XMODEM_START_WAIT_RCV;
                }
            }
            break;
        case ENUM_XMODEM_START_WAIT_RCV:
            //等待返回

            //超时
            if(!gl_bms_reboot_wait_cnt)
            {
                //gl_xmodem_state[bms_index] = ENUM_XMODEM_START;

                //gl_xmodem_state[bms_index] = ENUM_XMODEM_IDLE;
                //gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_IDLE;
                //gl_bms_update_state[bms_index] = ENUM_BMS_UPDATE_IDLE;
                //slave_rs485_cmd_update_start_cnf(ENUM_COMM_ERROR_CODE_TIMEOUT);
//                slave_rs485_cmd_update_return_error(bms_index, SLAVE_RS485_CMD_UPDATE_START, ENUM_COMM_ERROR_CODE_TIMEOUT);
            }
            break;
        case ENUM_XMODEM_TRANS_DATA:
            if(gl_update_buf.data_valid_flag)
            {
                gl_update_buf.data_head[0] = SOH;
                gl_update_buf.data_head[1] = gl_update_buf.data_offset / 128;
                gl_update_buf.data_head[1]++;
                gl_update_buf.data_head[2] = ~gl_update_buf.data_head[1];

                //crc
                {
                    unsigned short ccrc = crc16_ccitt((const char *)gl_update_buf.data_buf, 128);
                    gl_update_buf.data_crc16[0] = (ccrc>>8) & 0xFF;
                    gl_update_buf.data_crc16[1] = ccrc & 0xFF;
                }
                if(MM_snd_update_packet(bms_index, MM_ADDR_VALUE, gl_update_buf.data_head, 3+128+2))
                {
                    gl_bms_reboot_wait_cnt = XMODEM_TIMEOUT_MAX;
                    gl_xmodem_state[bms_index] = ENUM_XMODEM_TRANS_DATA_WAIT_RCV;
                }
            }
            break;
        case ENUM_XMODEM_TRANS_DATA_WAIT_RCV:

            

            //等待超时
            if(!gl_bms_reboot_wait_cnt)
            {
                gl_xmodem_state[bms_index] = ENUM_XMODEM_TRANS_DATA;
                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_TIMEOUT);
                gl_update_buf.data_valid_flag = 0;
            }
            break;
        case ENUM_XMODEM_END:

            //发送升级完成字节给BMS
            {
                unsigned char vl_data_tmp;
                vl_data_tmp = EOT;
                if(MM_snd_update_packet(bms_index, MM_ADDR_VALUE, &vl_data_tmp, 1))
                {
                    gl_bms_reboot_wait_cnt = XMODEM_TIMEOUT_MAX;
                    gl_xmodem_state[bms_index] = ENUM_XMODEM_END_WAIT_RCV;
                }
            }
            
            break;
        case ENUM_XMODEM_END_WAIT_RCV:

            //等待返回

            //等待超时
            if(!gl_bms_reboot_wait_cnt)
            {
                //gl_xmodem_state[bms_index] = ENUM_XMODEM_END;
                
                gl_xmodem_state[bms_index] = ENUM_XMODEM_TRANS_DATA;
                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_TIMEOUT);
                gl_update_buf.data_valid_flag = 0;
            }
                
            break;
        case ENUM_XMODEM_REBOOT:
            //XMODEM流程已经发送完成，发送重启命令给bms
            {
                unsigned char vl_tmp;
                //发送'U'给BMS
                vl_tmp = 'B';
                if(MM_snd_update_packet(bms_index, MM_ADDR_VALUE, &vl_tmp, 1))
                {
                    gl_bms_reboot_wait_cnt = 1500;
                    gl_xmodem_state[bms_index] = ENUM_XMODEM_REBOOT_WAIT_RCV;
                }
            }
            break;
        case ENUM_XMODEM_REBOOT_WAIT_RCV:

            //等待超时
            if(!gl_bms_reboot_wait_cnt)
            {
                //gl_xmodem_state[bms_index] = ENUM_XMODEM_END;
                
//                slave_rs485_cmd_update_return_error(gl_NTAGTX_param.vl_BMS_index, SLAVE_RS485_CMD_UPDATE_DONE, ENUM_COMM_ERROR_CODE_FAILED);
                
                gl_update_buf.data_valid_flag = 0;
            }
            
            break;    
        default:
            break;
    }
}




//重启并检查bms是否在bootloader状态
void bms_state_check(unsigned char bms_index)
{
    switch(gl_bms_state_check[bms_index])
    {
        case ENUM_BMS_STATE_CHECK_IDLE:            
            
            break;
        case ENUM_BMS_STATE_CHECK_REBOOT_BMS:
            //重启BMS，发送重启数据包
            {
                gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_W_RESET] = 1;
                gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_REBOOT_BMS_WAIT_RCV;

                gl_bms_reboot_wait_cnt = 1500;
            }
            
            break;
        case ENUM_BMS_STATE_CHECK_REBOOT_BMS_WAIT_RCV:
            //等待1.5S
            if(!gl_bms_reboot_wait_cnt)
            {
                //等待BMS重启前返回;
                //gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_SND;    

                gl_bms_update_state[bms_index] = ENUM_BMS_UPDATE_XMODEM_PROCESS;
                gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_IDLE;
                gl_xmodem_state[bms_index] = ENUM_XMODEM_START;
            }
            
            break;
        #if 0
        case ENUM_BMS_STATE_CHECK_SND:
            //发送任意字节给BMS
            {
                unsigned char vl_data_tmp;
                vl_data_tmp = 'V';
                if(MM_snd_update_packet(bms_index, MM_ADDR_VALUE, &vl_data_tmp, 1))
                {
                    gl_bms_reboot_wait_cnt = 1500;
                    gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_RCV;
                }
            }
            break;
        case ENUM_BMS_STATE_CHECK_RCV:
            //等待BMS返回
            if(!gl_bms_reboot_wait_cnt)
            {
                //超时
                gl_bms_state_check[bms_index] = ENUM_BMS_STATE_CHECK_REBOOT_BMS;//ENUM_BMS_STATE_CHECK_SND;
            }
            break;
        #endif
        default:
            break;
    }
}




//如果在升级状态，不再发送其他数据包
enum_bms_update_state bms_update_get_state(unsigned char bms_index)
{
    return gl_bms_update_state[bms_index];
}

/*

pl_hub_cmd: 如果在接收485通讯数据时候调用，该参数有效，不为0。
                    其他情况直接赋值NULL。
NFC_rcv_data:如果NFC接收到数据时候调用，该参数有效，不为0  。
                    其他情况直接赋值NULL
*/
void bms_update_process(unsigned char bms_index)
{
    switch(gl_bms_update_state[bms_index])
    {
        case ENUM_BMS_UPDATE_IDLE:
            break;
        case ENUM_BMS_UPDATE_CHECK_PROCESS:
            bms_state_check(bms_index);
            break;
        case ENUM_BMS_UPDATE_XMODEM_PROCESS:
            xmodem_transmit(bms_index);
            break;
        default:
            break;
    }
}

void bms_update_init(void)
{
    unsigned char i;
    for(i = 0; i < NFC_READER_COUNT_MAX; i++)
    {
        gl_bms_update_state[i] = ENUM_BMS_UPDATE_IDLE;
        gl_bms_state_check[i] = ENUM_BMS_STATE_CHECK_IDLE;
        gl_xmodem_state[i] = ENUM_XMODEM_IDLE;
        gl_bms_return_to_normal_mode_cnt = 0;
    }
}

void bms_update_timer_cb(void)
{
    if(gl_bms_reboot_wait_cnt)
        gl_bms_reboot_wait_cnt--;
    if(gl_bms_return_to_normal_mode_cnt)
    {
        gl_bms_return_to_normal_mode_cnt--;
        if(gl_bms_return_to_normal_mode_cnt == 0)
        {
            //因为PMS只管理一个仓，所以默认bms_index为0
            gl_xmodem_state[0] = ENUM_XMODEM_IDLE;
            gl_bms_state_check[0] = ENUM_BMS_STATE_CHECK_IDLE;
            gl_bms_update_state[0] = ENUM_BMS_UPDATE_IDLE;
        }
    }
}

void bms_xmodem_total_timeout_cnt_reload(void)
{
    gl_bms_return_to_normal_mode_cnt = BMS_XMODEM_TOTAL_TIMEOUT_MAX;
}
