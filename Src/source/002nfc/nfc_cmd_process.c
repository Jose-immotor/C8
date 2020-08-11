

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

////extern unsigned char vl_BMS_index; //��0��ʼ

const st_nfc_cmd_table gl_nfc_cmd_table[] = {
    //�����ּĴ���
    {MM_FN_CODE_READ_REG, MM_read_reg_rsp_cb, MM_read_reg_err_cb},
    //д�����Ĵ���
    {MM_FN_CODE_WRITE_REG, MM_write_reg_rsp_cb, MM_write_reg_err_cb},
    //д����Ĵ���
    {MM_FN_CODE_WRITE_MULTI_REG, MM_write_multi_reg_rsp_cb, MM_write_multi_reg_err_cb},
    //����BMS������
//    {MM_FN_CODE_UPDATE_REG, MM_snd_update_packet_cb, MM_snd_update_packet_err_cb},
    #ifdef BAT_PMS_AUTHORITY_FUNCTION_EN
    //��֤
    {MM_FN_CODE_AUTHORITY_REG, MM_authority_rsp_cb, MM_authority_err_cb},
    #endif
    {MM_FN_CODE_READ_HR_REG, MM_history_record_rsp_cb, MM_history_record_err_cb},
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
��Ӧ�����
return : 0, ����1,��ȷ
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
            //�˳���ѯ����
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
    //֡�����Ӧ��ʱ
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
            //�˳���ѯ����
            break;
        }
    }
    return 1;
}

//��ȡ�Ĵ�����ַ�ڴ洢���е�λ��
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

    //��Ϊû�л��Ʊ�֤���ҵļĴ�����ַһ���ڴ洢��Χ�ڣ�
    //���Է������һ���û����ݵ�λ��
    return BMS_MODBUS_REG_MAX-1;
    
}

/*
return: 0, �������; 1, ������ȷ
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
        //�ӻ����س��ִ������Ӧ
        
        return 1;
    }

    //�������2�ı��������ش���
    if(pl_rcv_buff[5]%2)
    {
        return 0;
    }

    //�жϷ��͵������Ƿ��Ѿ������
    if(snd_buf_len < 7)
    {
        return 0;
    }
    //�Ĵ����Ŀ�ʼ��ַ
    vl_reg_begin_addr = pl_snd_buff[5]<<8;
    vl_reg_begin_addr += pl_snd_buff[6];

    //�������������ctrl�Ĵ��������ٴ�����
    if((vl_reg_begin_addr == CTRL_REG_BIGIN_ADDR)&&
        (gl_cmd_buf[gl_NTAGTX_param.vl_BMS_index][ENUM_NFC_CMD_INDEX_W_CTRL]))
    {
        return 1;
    }
    
    i = MM_get_reg_addr_position_in_table(vl_reg_begin_addr);
    if(i == BMS_MODBUS_REG_MAX) //Խ�磬���ش���
        return 0;
    memcpy(&(gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->reg_unit[i]), &pl_rcv_buff[6], pl_rcv_buff[5]);  

	#ifdef BAT_PMS_AUTHORITY_FUNCTION_EN
	//�����֤��־λ����λ
	if((1 << 8) & Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_STATE))
	{
		// �����Ҫ��֤
		//��Ҫ��֤������BAT��֤PMSͨ�����ң�������PMS��֤BAT����PMS��֤BATͨ����
		if((Is_Authority_done_about_bat_pms()) && 
			((0 == Authority_get_onoff_about_pms_bat()) ||
			(Is_Authority_done_about_pms_bat())))
		{
			Authority_reset();
		}
	}
	else
	{
		// ����Ѿ���֤��������Ҫ����֤
		Authority_set_done_flag_about_bat_pms();
	}
	#endif
   
    
    return 1;
}
/*
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_read_reg_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    //֡�����Ӧ��ʱ
    //���³�ʼ��NFC����
    //NTAG_task_reset();
    
    return 1;
}
/*
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_write_reg_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //�ӻ����س��ִ������Ӧ
        return 1;
    }

    if(0 != memcmp(pl_rcv_buff, pl_snd_buff,snd_buf_len))
    {
        //�����д�
        return 0; 
    }
    return 1;
}
/*
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_write_reg_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    return 1;
}
/*
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_write_multi_reg_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //�ӻ����س��ִ������Ӧ
        return 1;
    }

    if(0 != memcmp(pl_rcv_buff, pl_snd_buff,9))
    {
        //�����д�
        return 0; 
    }
    return 1;
}
/*
return: 0, �������; 1, ������ȷ
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
//NFCӦ��������
//return: 0, �������; 1, ������ȷ
//*/
//st_update_buf gl_update_buf;
//unsigned char MM_snd_update_packet_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    if(pl_rcv_buff[4]&0x80)
//    {
//        //�ӻ����س��ִ������Ӧ

//        #ifdef BAT_ERROR_CHECK_DELAY_AFTER_BAT_UPDATE
//        if(gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode)
//        {
//            sl_bat_error_check_delay_after_bat_update_cnt = 120000; // 120��
//            //����������Ե�ع��ϵ�����
//            Battery_check_bat_clear_on_and_error();
//        }
//        #endif

//        //��ʶ����APP״̬
//        gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 0;
//        return 1;
//    }

//    //��ʶ����bootloader״̬
//    gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 1;
//    
//    //�����������״̬
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
//        //̽���Ƿ���bootloaderģʽ
//        //��ȷ����
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
//        //�ȴ�����
//        switch (gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index][5])
//        {
//            case ACK:
//                //bms������ȷӦ��
//                //++packetno;
//                //len += bufsz;
//                //goto start_trans;

//                gl_update_buf.data_valid_flag = 0;
//                gl_xmodem_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_XMODEM_TRANS_DATA;

//                slave_rs485_cmd_update_cnf(2, SLAVE_RS485_CMD_UPDATE_EXCHANGE, ENUM_COMM_ERROR_CODE_OK);
//                break;
//            case CAN:
//                //bms�ܾ���ֹͣ����
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
//                //bms�ܾ���ֹͣ����
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
//                sl_bat_error_check_delay_after_bat_update_cnt = 120000; // 120��
//                //����������Ե�ع��ϵ�����
//                Battery_check_bat_clear_on_and_error();
//            }
//            #endif
//            //��ʶ�˳�bootloader״̬
//            gl_bms_app_bl_status[gl_NTAGTX_param.vl_BMS_index].is_bl_mode = 0;

//            //��������
////            Battery_cmd_buf_clear(vl_BMS_index);
////            gl_bms_info_p[vl_BMS_index]->online = 0;
////            //����Ĵ�����Ч�ı�־
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
//NFCӦ��������
//return: 0, �������; 1, ������ȷ
//*/
//unsigned char MM_snd_update_packet_err_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    if(pl_rcv_buff[4]&0x80)
//    {
//        //�ӻ����س��ִ������Ӧ
//        return 1;
//    }

//    return 1;
//}


////�������ݰ���BMS
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

//    //��ս��ջ���
//    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
//    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
//    //gl_modbus_param.MM_NFC_snd_len
//    //gl_modbus_param.MM_NFC_snd_buff
//    return 1;
//}



/*
NFCӦ����֤��
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_history_record_rsp_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //�ӻ����س��ִ������Ӧ        
        return 1;
    }

//    Battery_history_record_set_data(&pl_rcv_buff[5]);
    
    return 1;
}


/*
NFCӦ��������
return: 0, �������; 1, ������ȷ
*/
unsigned char MM_history_record_err_cb(unsigned char *pl_rcv_buff,
                                        unsigned char rcv_buf_len,
                                        unsigned char *pl_snd_buff,
                                        unsigned char snd_buf_len)
{
    if(pl_rcv_buff[4]&0x80)
    {
        //�ӻ����س��ִ������Ӧ
        return 1;
    }

    return 1;
}


//#ifdef BAT_PMS_AUTHORITY_FUNCTION_EN

//typedef struct {
//	unsigned char cmd;
//	bool (*bat_authority_cmd_process_func)(st_hub_cmd *);
//}st_bat_authority_cmd;

//enum{
//ENUM_BAT_AUTHORITY_CMD_BAT_PMS_REQ = 1, // BAT ��֤PMS������
//ENUM_BAT_AUTHORITY_CMD_BAT_PMS_DIGEST = 2, // BAT��֤PMS��PMS������ϢժҪ��BAT��֤
//ENUM_BAT_AUTHORITY_CMD_PMS_BAT_REQ = 3, // PMS��֤BAT����BAT������ϢժҪ��PMS��֤
//};

////����������֤��ʱ����
//#define AUTHORITY_REACTION_INTERVAL_MAX (1000*60*1) // 1����

//typedef struct{
//unsigned int randomNB_from_bat;
//unsigned int randomNB_from_pms;
//unsigned int reauthority_delay_cnt; // ������֤�ļ�ʱ��ÿ��һ��ʱ�����֤һ��
//unsigned char bat_pms_authority_result; // BAT��֤PMS�Ľ��
//unsigned char pms_bat_authority_result; // PMS��֤BAT�Ľ��
//unsigned char bat_pms_reauthority_flag; // ���½���BAT��֤PMS
//unsigned char pms_bat_reauthority_flag; // ���½���PMS��֤BAT
//unsigned char bat_pms_authority_step; // BAT��֤PMS������
//unsigned char pms_bat_authority_step; // PMS��֤BAT������
//// ��֤�����Ƿ����õı�־��������������Ҫ��֤�����ǻ�����֤���̣�ֻ�ǲ����Ƴ�ŵ�
//unsigned char authority_enable_flag; 
//}st_authority_param;

//st_authority_param gl_authority_param[NFC_READER_COUNT_MAX];

//const char *sl_authority_secret = "sb.l34&@,alwvc9^t!sUDRGUN427zosp";


//st_bat_authority_cmd gl_bat_authority_cmd[] = {
//{ENUM_BAT_AUTHORITY_CMD_BAT_PMS_REQ, Authority_bat_pms_rsp},
//{ENUM_BAT_AUTHORITY_CMD_BAT_PMS_DIGEST, Authority_bat_pms_digest_rsp},
//{ENUM_BAT_AUTHORITY_CMD_PMS_BAT_REQ, Authority_pms_bat_rsp}
//};

////�ú�����Ҫ������main_pms_param_in_flash_init()����
//void Authority_init(void)
//{
//    unsigned char authority_enable_flag_old;
//    authority_enable_flag_old = gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag;
//    memset((char*)&gl_authority_param[gl_NTAGTX_param.vl_BMS_index], 0, sizeof(st_authority_param));
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].reauthority_delay_cnt = AUTHORITY_REACTION_INTERVAL_MAX;
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag = authority_enable_flag_old;
//}

//void Authority_reset(void)
//{
//    unsigned char authority_enable_flag_old;
//    authority_enable_flag_old = gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag;
//    Authority_init();
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag = authority_enable_flag_old;
//}

//void Authority_get_digest(unsigned int random_num, unsigned char * out_data)
//{
//    
//    unsigned char *pl_temp1;
//    unsigned char *pl_temp2;
//    unsigned int vl_random_num;
//    unsigned short vl_short_temp;
//    unsigned char vl_input[32];

//    memcpy((char*)vl_input, sl_authority_secret, 32);
//    vl_random_num = random_num;
//    pl_temp1 = (unsigned char*)&vl_random_num;
//    pl_temp2 = (unsigned char*)&vl_short_temp;
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_SN12);
//    vl_input[0] += (pl_temp1[2] + pl_temp2[0]);
//    vl_input[1] += (pl_temp1[1] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_MCUN56);
//    vl_input[2] += (pl_temp1[4] + pl_temp2[0]);
//    vl_input[3] += (pl_temp1[3] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_MCUN12);
//    vl_input[4] += (pl_temp1[2] + pl_temp2[0]);
//    vl_input[5] += (pl_temp1[1] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_SN34);
//    vl_input[6] += (pl_temp1[4] + pl_temp2[0]);
//    vl_input[7] += (pl_temp1[3] + pl_temp2[1]);

//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_MCUN34);
//    vl_input[20] += (pl_temp1[2] + pl_temp2[0]);
//    vl_input[21] += (pl_temp1[1] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_SN78);
//    vl_input[22] += (pl_temp1[4] + pl_temp2[0]);
//    vl_input[23] += (pl_temp1[3] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_SN56);
//    vl_input[24] += (pl_temp1[2] + pl_temp2[0]);
//    vl_input[25] += (pl_temp1[1] + pl_temp2[1]);
//    vl_short_temp = Battery_get_reg_value(gl_NTAGTX_param.vl_BMS_index, ENUM_REG_ADDR_MCUN78);
//    vl_input[30] += (pl_temp1[4] + pl_temp2[0]);
//    vl_input[31] += (pl_temp1[3] + pl_temp2[1]);
//    
//    //mbedtls_sha256((const unsigned char * )vl_input, 32, out_data, 0);
//    hmac_sha256((const unsigned char * )vl_input, 32,(const unsigned char * )sl_authority_secret, 32, out_data);
//}

//static unsigned char Authority_bat_pms_req(void)
//{
//    unsigned char vl_buf[3];
//    vl_buf[0] = ENUM_BAT_AUTHORITY_CMD_BAT_PMS_REQ;
//    vl_buf[1] = 1;
//    vl_buf[2] = 0;
//    return MM_snd_authority_data(MM_ADDR_VALUE, vl_buf, 3);
//}

//bool Authority_bat_pms_rsp(st_hub_cmd *pl_hub_cmd)
//{
//    if(pl_hub_cmd->stl_data[0])
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_bat = (pl_hub_cmd->stl_data[4] 
//                                        + (pl_hub_cmd->stl_data[3] << 8)
//                                        + (pl_hub_cmd->stl_data[2] << 16)
//                                        + (pl_hub_cmd->stl_data[1] << 24));
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_step = 1;
//    }
//    else
//    {
//        
//    }

//    return TRUE;
//}
//static unsigned char Authority_bat_pms_digest_req(void)
//{
//    unsigned char vl_buf[34];
//    
//    vl_buf[0] = ENUM_BAT_AUTHORITY_CMD_BAT_PMS_DIGEST;
//    vl_buf[1] = 32;
//    //����Digest
//    Authority_get_digest(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_bat, &vl_buf[2]);
//    return MM_snd_authority_data(MM_ADDR_VALUE, vl_buf, 34);
//}


//bool Authority_bat_pms_digest_rsp(st_hub_cmd *pl_hub_cmd)
//{
//    if(pl_hub_cmd->stl_data[0])
//    {
//        //��֤�ɹ�
//        Authority_set_done_flag_about_bat_pms();
//    }

//    return TRUE;
//}

//static unsigned char Authority_pms_bat_req(void)
//{
//    unsigned char vl_buf[6];
//    vl_buf[0] = ENUM_BAT_AUTHORITY_CMD_PMS_BAT_REQ;
//    vl_buf[1] = 4;
//    vl_buf[2] = (gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_pms>>24)&0xFF;
//    vl_buf[3] = (gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_pms>>16)&0xFF;
//    vl_buf[4] = (gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_pms>>8)&0xFF;
//    vl_buf[5] = gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_pms&0xFF;
//    
//    return MM_snd_authority_data(MM_ADDR_VALUE, vl_buf, 6);
//}

//bool Authority_pms_bat_rsp(st_hub_cmd *pl_hub_cmd)
//{
//    unsigned char vl_digest_buff[32];
//    // ����Digest
//    //���ƥ�� 
//    //gl_authority_param.pms_bat_authority_result = 1;
//    Authority_get_digest(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].randomNB_from_pms,vl_digest_buff);
//    if((1 == pl_hub_cmd->stl_data[0])&&
//        (0 == memcmp((char*)vl_digest_buff, (char*)&pl_hub_cmd->stl_data[1], 32)))
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_result = 1;
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_step = 1;
//    }
//    else
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_result = 0;
//    }

//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_reauthority_flag = 0;
//    return TRUE;
//}

//bool Authority_NFC_cmd_process(st_hub_cmd *pl_hub_cmd)
//{
//    unsigned int i, vl_cmd_cnt;
//    vl_cmd_cnt = sizeof(gl_bat_authority_cmd)/sizeof(st_bat_authority_cmd);
//    for(i = 0; i < vl_cmd_cnt; i++)
//    {
//        if(pl_hub_cmd->stl_cmd == gl_bat_authority_cmd[i].cmd)
//        {
//            return gl_bat_authority_cmd[i].bat_authority_cmd_process_func(pl_hub_cmd);
//        }
//    }
//    return FALSE;
//}


///*
//NFCӦ����֤��
//return: 0, �������; 1, ������ȷ
//*/
//unsigned char MM_authority_rsp_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    st_hub_cmd vl_hub_cmd;

//    if(pl_rcv_buff[4]&0x80)
//    {
//        //�ӻ����س��ִ������Ӧ        
//        return 1;
//    }

//    //��֤
//    vl_hub_cmd.stl_cmd = pl_rcv_buff[5];
//    vl_hub_cmd.stl_dataL = pl_rcv_buff[6];
//    vl_hub_cmd.stl_data = &pl_rcv_buff[7];
//    
//    return Authority_NFC_cmd_process(&vl_hub_cmd);
//}


///*
//NFCӦ��������
//return: 0, �������; 1, ������ȷ
//*/
//unsigned char MM_authority_err_cb(unsigned char *pl_rcv_buff,
//                                        unsigned char rcv_buf_len,
//                                        unsigned char *pl_snd_buff,
//                                        unsigned char snd_buf_len)
//{
//    if(pl_rcv_buff[4]&0x80)
//    {
//        //�ӻ����س��ִ������Ӧ
//        return 1;
//    }

//    return 1;
//}

////�������ݰ���BMS
//unsigned char MM_snd_authority_data(unsigned char slave_addr,unsigned char* data, unsigned int data_len)
//{
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_PCB_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_RESERVE_VALUE;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = slave_addr;
//    gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]++] = MM_FN_CODE_AUTHORITY_REG;
//    memcpy(&gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index][gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]], (unsigned char*)data, data_len);
//    gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] +=data_len;

//    gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index] = slave_addr;
//    gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index] = MM_FN_CODE_AUTHORITY_REG;

//    //��ս��ջ���
//    gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
//    memset(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 0x00, MM_RCV_BUFF_MAX);
//    //gl_modbus_param.MM_NFC_snd_len
//    //gl_modbus_param.MM_NFC_snd_buff
//    return 1;
//}

//unsigned char MM_snd_authority_process(void)
//{
//    if((!gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_result) || (gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_reauthority_flag))
//    {
//        //BAT��Ҫ��֤PMS
//        switch(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_step)
//        {
//            case 0:
//                return Authority_bat_pms_req();
//                break;
//            case 1:
//                return Authority_bat_pms_digest_req();
//                break;
//            default:
//                break;
//        }
//        
//    }
//    else if(0 != Authority_get_onoff_about_pms_bat())
//    {
//        // PMS ��Ҫ��֤BAT
//        switch(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_step)
//        {
//            case 0:
//                return Authority_pms_bat_req();
//                break;
//            default:
//                break;
//        }
//    }

//    return 1;
//}

//// �ں���Battery_info_polling_Process()�е���
//void Authority_process(void)
//{
//    
//    //�������豸��Ϣ�ļĴ������������������Ч
//    //�жϵ�ص���֤λ�Ƿ���λ
//	//��ز���֤�����������Ч���� �������֤PMS��ɲ���PMS��֤�����ɣ��������½���BAT��֤PMS ���ң����½���PMS��֤BAT����������PMS��֤BAT��
//    if((0 == slave_rs485_is_bat_valid(gl_NTAGTX_param.vl_BMS_index)) 
//     ||(((Is_Authority_done_about_bat_pms())&&((0 == Authority_get_onoff_about_pms_bat())||(Is_Authority_done_about_pms_bat())))
//	  &&(0 == gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_reauthority_flag)
//	  &&((0 == gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_reauthority_flag) || (0 == Authority_get_onoff_about_pms_bat())))
//       )
//    {
//        return;
//    }

//    
//    //��ص͹��Ĺ��������ѯ�ڼ䣬ͬʱ��о��͵�ѹС��3V����ֹPMS��֤���
//    if((0 != sl_bat_poll_param.is_poll_allways_en_flag) //����һֱ��ѯ״̬
//        || (sl_bat_poll_param.poll_interval_MAX < BAT_POLL_BIG_INTERVAL_MAX))//��о��ѹ���ڵ���3V
//    {
//        gl_cmd_buf[gl_NTAGTX_param.vl_BMS_index][ENUM_NFC_CMD_INDEX_AUTHORITY] = 1;
//    }
//}


////�Ƿ�PMS��֤BATͨ��
//bool Is_Authority_done_about_pms_bat(void)
//{   
//    if(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_result)
//    {
//        return TRUE;
//    }
//    else
//    {
//        return FALSE;
//    }
//}

////�Ƿ�BAT��֤PMSͨ��
//bool Is_Authority_done_about_bat_pms(void)
//{
//    bool vl_result;
//    
//    if(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_result)
//    {
//        return TRUE;
//    }
//    else
//    {
//        return FALSE;
//    }
//}

////�Ƿ�PMS��֤BATͨ������BAT��֤PMSͨ��
//bool Is_Authority_done(void)
//{
//    bool vl_result;
//    
//    if((gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_result)
//        &&(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_result))
//    {
//        vl_result = TRUE;
//    }
//    else
//    {
//        vl_result = FALSE;
//    }

//    return vl_result;
//}



//void Authority_done_flag_clear(void)
//{
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_result = 0;
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_result = 0;
//}

//void Authority_timer_count_cb(void)
//{
//    if(gl_authority_param[gl_NTAGTX_param.vl_BMS_index].reauthority_delay_cnt)
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].reauthority_delay_cnt--;
//        if(0 == gl_authority_param[gl_NTAGTX_param.vl_BMS_index].reauthority_delay_cnt)
//        {
//            gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_reauthority_flag = 1;
//            if(0 != Authority_get_onoff_about_pms_bat())
//            {
//                gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_reauthority_flag = 1;
//            }
//            gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_step = 0;
//            gl_authority_param[gl_NTAGTX_param.vl_BMS_index].pms_bat_authority_step = 0;

//            gl_authority_param[gl_NTAGTX_param.vl_BMS_index].reauthority_delay_cnt = AUTHORITY_REACTION_INTERVAL_MAX;
//        }
//    }
//}

////����BAT��֤PMS��ɵı�־�Ͳ���
//void Authority_set_done_flag_about_bat_pms(void)
//{
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_result = 1;
//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_reauthority_flag = 0;

//    gl_authority_param[gl_NTAGTX_param.vl_BMS_index].bat_pms_authority_step = 2;
//}

//void Authority_set_onoff(unsigned char onoff)
//{
//    if(onoff)
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag = 1;
//    }
//    else
//    {
//        gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag = 0;
//    }

//    /*main_set_authority_param_to_flash();*/
//}

////�Ƿ�������PMS��֤BAT
//unsigned char Authority_get_onoff_about_pms_bat(void)
//{
//    return gl_authority_param[gl_NTAGTX_param.vl_BMS_index].authority_enable_flag;
//}

//#endif
