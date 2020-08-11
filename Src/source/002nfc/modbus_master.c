

//#define MODBUS_MASTER_C
#include <string.h>
#include "ntag_app.h"
#include "modbus_master.h"
#include "NFC_cmd_process.h"
#include "battery.h"

st_modbus_param gl_modbus_param;

/*
��ʼ��MODBUS

*/
void MM_init(unsigned char bms_index)
{
//    gl_modbus_param.mm_state[bms_index] = ENUM_MM_NFC_INIT;
    //gl_modbus_param.md_rcv_state = ENUM_MD_INIT;
    //gl_modbus_param.need2snd_data = 0;
    gl_modbus_param.slave_addr[bms_index] = 0;
    gl_modbus_param.MM_NFC_snd_len[bms_index] = 0;
    gl_modbus_param.MM_NFC_rcv_len[bms_index] = 0;
    //gl_modbus_param.MM_IDLE_timer_cnt = 0;
    //gl_modbus_param.MM_NFC_snd_enable = 0;
    gl_modbus_param.MM_NFC_snd_err_cnt[bms_index] = 0;
    //gl_modbus_param.MM_NFC_nead_reset = 0;
}

/*
�жϰ�ͷ
fn_code: ������
return: 0, ʧ�ܣ�1���ɹ�
*/
unsigned char MM_check_packet_head(unsigned char *pl_buff, unsigned char slave_addr,unsigned char fn_code)
{
    if((pl_buff[0] != MM_PCB_VALUE)||
        (pl_buff[1] != MM_RESERVE_VALUE)||
        (pl_buff[2] != MM_RESERVE_VALUE)||
        (pl_buff[3] != slave_addr)||
        (((pl_buff[4]&0x7F) != fn_code)&&(fn_code != MM_FN_CODE_BYPASS))) // �����͸��ģʽ�Ͳ����ж�fn_code
    {
        return 0;
    }

    return 1;
}

/*
������ݰ���CRC
pl_buff�е�������ֽ���CRC
return : 0,��ʧ�ܣ�1���ɹ�
*/
unsigned char MM_check_packet_CRC(unsigned char *pl_buff, unsigned char buf_len)
{
    return 1;
}

//return : 1, ���͵������Ѿ�������ϣ����߷���ʧ�ܣ�����ִ���¸����Ͳ�����
//              0, ���ͻ�û�����
unsigned char MM_is_snd_done(void)
{
    if(//(gl_modbus_param.mm_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_MM_IDLE)&&
        (gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] == 0))
    {
        return 1;
    }

    return 0;
}

unsigned char MM_is_snd_done_by_index(unsigned char bms_index)
{
    if(//(gl_modbus_param.mm_state[bms_index] == ENUM_MM_IDLE)&&
        (gl_modbus_param.MM_NFC_snd_len[bms_index] == 0))
    {
        return 1;
    }

    return 0;
}


/*
�������������
return : 0, ����ʧ��; 1,������
*/
unsigned char MM_snd_req_cmd(unsigned char slave_addr, unsigned char fn_code, unsigned char *data, unsigned char data_len)
{
    //if(!MM_is_snd_done())
//    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
//    {
//        return 0;
//    }
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = fn_code;
    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], data, data_len);
    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=data_len;

    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = fn_code;

    //��ս��ջ���
    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
    //gl_modbus_param.MM_NFC_snd_len
    //gl_modbus_param.MM_NFC_snd_buff
    return 1;
}


unsigned char MM_snd_multi_read_cmd(unsigned char slave_addr,st_read_reg_03* pl_read_reg_03)
{
    //if(!MM_is_snd_done())
//    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
//    {
//        return 0;
//    }
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_FN_CODE_READ_REG;
    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], (unsigned char*)pl_read_reg_03, sizeof(st_read_reg_03));
    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=sizeof(st_read_reg_03);

    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_READ_REG;

    //��ս��ջ���
    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
    //gl_modbus_param.MM_NFC_snd_len
    //gl_modbus_param.MM_NFC_snd_buff
    return 1;
}


unsigned char MM_snd_multi_write_cmd(unsigned char slave_addr,st_write_reg_10* pl_write_reg_10)
{
    //if(!MM_is_snd_done())
//    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
//    {
//        return 0;
//    }
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_FN_CODE_WRITE_MULTI_REG;
    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], (unsigned char*)pl_write_reg_10, 5);
    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=5;

    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], 
	                                        pl_write_reg_10->byte_data, pl_write_reg_10->byte_cnt);
    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=pl_write_reg_10->byte_cnt;

    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_WRITE_MULTI_REG;

    //��ս��ջ���
    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
    return 1;
}

//record_index: ��ʷ���ݼ�¼��
unsigned char MM_snd_read_history_record_cmd(unsigned char slave_addr, unsigned int record_index)
{
    //if(!MM_is_snd_done())
//    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
//    {
//        return 0;
//    }
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_FN_CODE_READ_HR_REG;
    //memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], (unsigned char*)pl_write_reg_10, 5);
    //gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=5;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = (record_index>>24)&0xFF;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = (record_index>>16)&0xFF;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = (record_index>>8)&0xFF;
    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = (record_index)&0xFF;


    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_READ_HR_REG;

    //��ս��ջ���
    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
    //gl_modbus_param.MM_NFC_snd_len
    //gl_modbus_param.MM_NFC_snd_buff
    return 1;
}

unsigned char MM_snd_bypass(unsigned char slave_addr,unsigned char* data, unsigned int data_len)
{
    //if(!MM_is_snd_done())
//    if(!MM_is_snd_done_by_index(gl_NTAGTX_param.vl_BMS_index))
//    {
//        return 0;
//    }
    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], data, data_len);
    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] += data_len;

    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_BYPASS;

    //��ս��ջ���
    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
    //gl_modbus_param.MM_NFC_snd_len
    //gl_modbus_param.MM_NFC_snd_buff
    return 1;
}


/*
NFCͨ����ʼ����ϣ��ص��ú���
*/
void MM_NFC_init_done(unsigned char bms_index)
{
//    if((gl_modbus_param.mm_state[bms_index] != ENUM_MM_NFC_INIT)&&
//        (gl_modbus_param.mm_state[bms_index] != ENUM_MM_IDLE))
//    {
//        return ;
//    }
//    
//    gl_modbus_param.mm_state[bms_index] = ENUM_MM_IDLE;
    gl_modbus_param.MM_NFC_snd_len[bms_index] = 0;
    gl_modbus_param.MM_NFC_rcv_len[bms_index] = 0;
}

/*
NFC����������ϣ��ص��ú���
*/
void MM_NFC_snd_done_cb(void)
{
    //gl_modbus_param.MM_NFC_snd_len = 0;
    gl_modbus_param.MM_NFC_snd_err_cnt[gl_NTAGTX_param.vl_BMS_index] = 0;

    //���NFC����
    gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->online = 1;
}

/*
����ʧ�ܣ��ص�����
return : 0, �ط�����û����1���ط�������
*/
unsigned char MM_NFC_snd_ERR(void)
{
    gl_modbus_param.MM_NFC_snd_err_cnt[gl_NTAGTX_param.vl_BMS_index]++;
    if(gl_modbus_param.MM_NFC_snd_err_cnt[gl_NTAGTX_param.vl_BMS_index] >= MM_NFC_SND_ERR_MAX)
    {
        gl_modbus_param.MM_NFC_snd_err_cnt[gl_NTAGTX_param.vl_BMS_index] = 0;

        //���NFC������, ����Ӧ���ö�ʱ����ʱ���жϵ���Ƿ�����
        gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->online = 0;
        Battery_cmd_buf_clear(gl_NTAGTX_param.vl_BMS_index);
        //gl_modbus_param.MM_NFC_snd_len = 0;
        return 1;
    }

    return 0;
}


///*
//1ms��ʱ���жϴ���ص�����
//*/
//void MM_timer_count_cb(void)
//{
////    if(gl_modbus_param.MM_IDLE_timer_cnt)
////        gl_modbus_param.MM_IDLE_timer_cnt--;
//}

///*
//���ø��ĸ�BMSͨѶ
//return : 0, ��ʾ����ʧ�ܣ�1����ʾ���óɹ�
//*/
//unsigned char MM_set_BMS_index(unsigned char pl_index)
//{
////    unsigned char i;
////    if(gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index])
////        return 0;//��һ������û�д������
//    
////    for(i = 0; i < NFC_READER_COUNT_MAX; i++)
////    {
////        if((gl_modbus_param.mm_state[i] != ENUM_MM_NFC_INIT)&&
////            (gl_modbus_param.mm_state[i] != ENUM_MM_IDLE))
////        {
////            
////            break;
////        }
////    }

////    if(i == NFC_READER_COUNT_MAX)
////    {
////        gl_NTAGTX_param.vl_BMS_index = pl_index;
////        return 1;
////    }
//    
//    return 1;
//    
//}

//unsigned char MM_is_bypass_fn_code_working(void)
//{
//    if(gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] == MM_FN_CODE_BYPASS)
//        return 1;
//    else
//        return 0;
//}

//unsigned char MM_is_read_HR_fn_code_working(void)
//{
//    if(gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] == MM_FN_CODE_READ_HR_REG)
//        return 1;
//    else
//        return 0;
//}

//void MM_task(void)
//{
//    
//}
