/*
 * File      : ntag_app.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-16     kim          the first version
 * 2020-04-04     lane         完善逻辑
 * ...
 */
//#define NTAG_APP_C
#include "fm175xx.h"
#include "ntag_app.h"
#include "battery.h"
#include "type_a.h"
#include "nfc_crc.h"
#include "nfc_cmd_process.h"
//#include "xmodem.h"
#include "modbus_master.h"

#define ERROR		1
#define	OK			0

//#ifdef PMS_IIC_ERROR_COUNT_ENABLE
////顺便在这里统计读电池失败次数
//unsigned int gl_NTAG_read_faild_cnt;
//unsigned int gl_NTAG_read_cnt;
////顺便在这里统计读电池成功的次数
//unsigned int gl_NTAG_read_sucess_cnt;
////顺便在这边统计读电池返回的数据CRC校验出错的次数
//unsigned int gl_NTAG_read_crc_err_cnt;
//#endif



st_fn_NTAGTX_param gl_NTAGTX_param;

unsigned char picc_ats[14];

unsigned long CID;                                         //如果没有用到CID，CID=0
unsigned char nfc_buf[16];
unsigned int nfc_bitlen;

//在开始传输NFC前置1，传输完成后置0
static unsigned char gl_ntag_nfc_sending_receiving_flag = 0;

///****************************************************************/
///*名称: TypeA_PPSS 													           */
///*功能: 该函数实现ISO14443A-4协议中的RATS处理			     */
///*输入: PPS0用于决定PPS1是否传输，PPS1    */ 			
///*       ats 用于保存RATS命令返回的数据                */
///*输出:															                   */
///* OK: 应答正确												                 */
///* ERROR: 应答错误												             */
///****************************************************************/


//unsigned char TypeA_PPSS(unsigned char Pps0, unsigned char Pps1, unsigned char *PpsAck)
//{
//    unsigned char send_buff[4];
//    unsigned char send_num;
//    unsigned char result;
//	unsigned int rece_bitlen;
//    
//    send_buff[0] = 0xD0;
//    send_num=1;
//    if(Pps0&0x10)                                        //如果需要改变传输速率
//    {
//        send_buff[1] = Pps0;
//        send_buff[2] = Pps1;
//        send_num += 2;
//    }
//    Write_Reg(BitFramingReg,0x00);
//    Set_BitMask(TxModeReg,0x80);                         //设置发送CRC

//    Set_BitMask(RxModeReg,0x80);                         //设置接收CRC
//    Clear_BitMask(Status2Reg,0x08);
//    Pcd_SetTimer(5);
//    Clear_FIFO();
//    result = Pcd_Comm(Transceive,send_buff,send_num,PpsAck,&rece_bitlen);
//    return result;
//}

/****************************************************************/
/*名称: TypeA_RATS 													           */
/*功能: 该函数实现ISO14443A-4协议中的RATS处理			     */
/*输入: param byte bit8-bit5：FSDI,bit4-bit0 ：CID     */ 			
/*       ats 用于保存RATS命令返回的数据                */
/*输出:															                   */
/* OK: 应答正确												                 */
/* ERROR: 应答错误												             */
/****************************************************************/
unsigned char TypeA_RATS(unsigned char param, unsigned char *ats)
{
    unsigned char send_buff[4];
    unsigned char result;
	unsigned int rece_bitlen;
    
    send_buff[0] = 0xE0;
    send_buff[1] = param;                                //
    CID = param&0x0f;
    Write_Reg(BitFramingReg,0x00);
    Set_BitMask(TxModeReg,0x80);                         //设置发送CRC
    Set_BitMask(RxModeReg,0x80);                         //设置接收CRC
    Clear_BitMask(Status2Reg,0x08);
    Pcd_SetTimer(5);
    Clear_FIFO();
    result = Pcd_Comm(Transceive,send_buff,2,ats,&rece_bitlen);
    return result;
}



/****************************************************************/
/*名称: FM11NC08Active 										    */
/*功能: 该函数实现ISO14443A-4协议RATS和PPSS 往后的数据就是透明传输*/
/*输入:  	ats RATS命令返回数据								*/
/*输出:															*/
/* OK: 应答正确													*/
/* ERROR: 应答错误												*/
/****************************************************************/
unsigned char FM11NC08Active(unsigned char *ats)
{
    unsigned char result;
    result=TypeA_RATS(0x20,ats);                                //0x20表示读卡器只能接收32字节长度的数据
    return result;                                              //如果需要接收更长帧的数据，可以修改读卡器接收函数，最长可以接收256字节
}

static unsigned char NTAG_rcv_CRC_check(unsigned char *pl_rcv_data, unsigned int pl_rcv_len)
{
    unsigned char vl_crc[2];
    unsigned char vl_rcv_crc[2];
    unsigned int vl_rcv_crc_pos;
    
    if(pl_rcv_len < 2)
        return 0;
    #if 0
    //crc 校验
    ComputeCrc(CRC_A, pl_rcv_data, (pl_rcv_len - 2), &vl_crc[0], &vl_crc[1]);
    if((pl_rcv_data[pl_rcv_len - 2] != vl_crc[0])||(pl_rcv_data[pl_rcv_len - 1] != vl_crc[1]))
    {
        return 0;
    }
    #else
    if(0x03 == pl_rcv_data[4])
    {
        /*是读寄存器命令*/

        vl_rcv_crc_pos = pl_rcv_data[5] + 6;
        vl_rcv_crc[0] = pl_rcv_data[vl_rcv_crc_pos];
        vl_rcv_crc[1] = pl_rcv_data[vl_rcv_crc_pos+1];

        //crc 校验
        ComputeCrc(CRC_A, pl_rcv_data, (pl_rcv_len - 2), &vl_crc[0], &vl_crc[1]);
        if((vl_rcv_crc[0] != vl_crc[0])||(vl_rcv_crc[1] != vl_crc[1]))
        {
            return 0;
        }
    }
    else
    {
        //crc 校验
        ComputeCrc(CRC_A, pl_rcv_data, (pl_rcv_len - 2), &vl_crc[0], &vl_crc[1]);
        if((pl_rcv_data[pl_rcv_len - 2] != vl_crc[0])||(pl_rcv_data[pl_rcv_len - 1] != vl_crc[1]))
        {
            return 0;
        }
    }
    
    #endif
    return 1;
    
}

//读卡器搜卡失败次数计算最大值
#define NTAG_FOUND_NFC_CNT_MAX 6 // 20191221
#define NTAG_FOUND_NFC_CNT_MAX_WHEN_ELOCK_OPEN 8
static unsigned char gl_ntag_found_nfc_cnt[NFC_READER_COUNT_MAX];
void NTAG_clear_reg_valid_flag(unsigned char bms_index)
{
    if(gl_ntag_found_nfc_cnt[bms_index] >= NTAG_FOUND_NFC_CNT_MAX)
    {
//        Battery_clear_reg_valid_flag(bms_index);
		gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->online = 0;
        gl_ntag_found_nfc_cnt[bms_index] = 0;
//		if(bms_index == 0)
//		{
//			//拉低电源
//			rt_pin_write(FM17522_ON, PIN_LOW);
//			FM17522_Delayms(200);
//			//拉高电源
//			rt_pin_write(FM17522_ON, PIN_HIGH);
//			FM17522_Delayms(200);
//		}
//		else if(bms_index == 1)
//		{
//			//拉低电源
//			rt_pin_write(FM17522_ON1, PIN_LOW);
//			FM17522_Delayms(200);
//			//拉高电源
//			rt_pin_write(FM17522_ON1, PIN_HIGH);
//			FM17522_Delayms(200);
//		}
	
    }
    else
    {
        gl_ntag_found_nfc_cnt[bms_index]++;
    }
}
//检测NFC通讯计时，如果没有通讯10秒后，重新初始化
#define NFC_COMM_IDLE_CNT_MAX 10000 //10s
unsigned char NTAG_task_timer_cb(void)
{
    if(gl_NTAGTX_param.vl_interval_timer_cnt)
    {
        gl_NTAGTX_param.vl_interval_timer_cnt--;
        if(gl_NTAGTX_param.vl_interval_timer_cnt == 0)
        {
            return 1;
        }
    }
	return 0;
}
//NFC通讯成功就刷新一次
void NTAG_task_timer_reload(void)
{
    gl_NTAGTX_param.vl_interval_timer_cnt = NFC_COMM_IDLE_CNT_MAX;
}
void Battery_info_polling_Process(unsigned char bms_index)
{
	#ifdef OLDER_CODE
    //开机查询一次电池版本信息
    //if(vl_bat_info_flag[0]==0)
    {
        gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_VERSION] = 1;
    }
    
    //查询BMS 只读信息，从寄存器256到308
    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_INFO1] = 1;
    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_INFO2] = 1;
    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_CTRL] = 1;

    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_PARAM1] = 1;
    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_PARAM2] = 1;
 

    //开机查询一次
    {
        gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_USERDATA] = 1;
    }

//    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_R_BLV] = 1;

    #ifdef BAT_PMS_AUTHORITY_FUNCTION_EN
    //电池双向认证流程
    Authority_process();
    #endif
	#endif
    //读取电池历史记录数据
//    if((0 == gl_bat_history_record_param.is_earliest_param_valid)
//        ||(0 == gl_bat_history_record_param.is_lastest_param_valid))
//    {
//        gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_HISTORY_RECORD] = 1;
//    }
    
    return ;
}
typedef enum {
ENUM_NTAG_STATE_INIT,
ENUM_NTAG_STATE_SEARCH,    
ENUM_NTAG_STATE_TRANS

}enum_ntag_state;

//天线选择
unsigned char gl_NTAG_task_nfc_ant_selc[NFC_READER_COUNT_MAX];
enum_ntag_state gl_NTAG_task_state[NFC_READER_COUNT_MAX];//={ENUM_NTAG_STATE_INIT, ENUM_NTAG_STATE_INIT};

unsigned char NTAG_task_x(void)
{
    unsigned char result;
    unsigned int vl_tmp_cnt;
#if 0
    if(gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_NTAG_STATE_INIT)
    {
        //清除电池在线标志
        NTAG_clear_reg_valid_flag(gl_NTAGTX_param.vl_BMS_index);       
        FM175XX_HardReset();
        FM175XX_SoftReset();
        if(gl_NTAG_task_nfc_ant_selc[gl_NTAGTX_param.vl_BMS_index])
        {
            Set_Rf(1);
        }
        else
        {
            Set_Rf(2);
        }
        Pcd_ConfigISOType(0);
        gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_NTAG_STATE_SEARCH;
    }
    
    if(gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_NTAG_STATE_SEARCH)
    {        
        result = TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK);
        if (result==OK)
        {
            result = FM11NC08Active(picc_ats);
            if(result == OK)
            {
                //搜卡成功
                MM_NFC_init_done(gl_NTAGTX_param.vl_BMS_index);
                FM17522_Delayms(10);//因为卡片的程序中加了延迟，这里也相应延迟一下

                //读取电池信息
                Battery_info_polling_Process(gl_NTAGTX_param.vl_BMS_index);
                gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_NTAG_STATE_TRANS;
                FM17522_Delayms(10);
                //复位读卡失败计算
                gl_ntag_found_nfc_cnt[gl_NTAGTX_param.vl_BMS_index] = 0;
            }
            else
            {
                //清除命令缓存
                Battery_cmd_buf_clear(gl_NTAGTX_param.vl_BMS_index);
                //清除寄存器有效的标志
                NTAG_clear_reg_valid_flag(gl_NTAGTX_param.vl_BMS_index);
                gl_NTAG_task_nfc_ant_selc[gl_NTAGTX_param.vl_BMS_index] = ~gl_NTAG_task_nfc_ant_selc[gl_NTAGTX_param.vl_BMS_index];
                if (0 == Battery_get_reg_valid_flag(gl_NTAGTX_param.vl_BMS_index))
                {
                    //进入低功耗
                    FM175XX_SoftPowerdown();
                    //重新初始化读卡器任务
                    NTAG_task_reset();
                }
                return ERROR;
            }
        }
        else
        {
            //清除命令缓存
            Battery_cmd_buf_clear(gl_NTAGTX_param.vl_BMS_index);
            //清除寄存器有效的标志
            NTAG_clear_reg_valid_flag(gl_NTAGTX_param.vl_BMS_index);
            gl_NTAG_task_nfc_ant_selc[gl_NTAGTX_param.vl_BMS_index] = ~gl_NTAG_task_nfc_ant_selc[gl_NTAGTX_param.vl_BMS_index];
            if (0 == Battery_get_reg_valid_flag(gl_NTAGTX_param.vl_BMS_index))
            {
                //进入低功耗
                FM175XX_SoftPowerdown();
                //重新初始化读卡器任务
                NTAG_task_reset();
            }
            return ERROR;
        }
    }

    if(gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] == ENUM_NTAG_STATE_TRANS)
    {
		if((Battery_is_one_cmd_buf_empty(gl_NTAGTX_param.vl_BMS_index))
		 ||(0 == Battery_send_cmd(gl_NTAGTX_param.vl_BMS_index)))//填充要发送的数据
		 //||(gl_NTAGTX_param.vl_BMS_index != gl_NTAGTX_param.vl_BMS_index))//判断是否是该读卡器发送数据
        {
            //进入低功耗
            FM175XX_SoftPowerdown();
			//重新初始化读卡器任务
            NTAG_task_reset();
            return OK;
        }       
   //     for(;;)
   //     {
   //         if(!gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index])
   //         {
   //             //进入低功耗
   //             FM175XX_SoftPowerdown();
			//    //重新初始化读卡器任务
   //             NTAG_task_reset();
   //             return OK;
   //         }
   //         gl_ntag_nfc_sending_receiving_flag = 1;
   //         gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
   //         Pcd_SetTimer(300);
   //         result = Pcd_Comm(Transceive,
   //                           gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index],
   //                           gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index],
   //                           gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index],
   //                           &vl_tmp_cnt);
   //         gl_ntag_nfc_sending_receiving_flag = 0;
   //         #ifdef PMS_IIC_ERROR_COUNT_ENABLE
   //         //顺便在这里统计读电池次数
   //         gl_NTAG_read_cnt++;
   //         #endif
			//gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = (vl_tmp_cnt+7)/8;
   //         if(result == OK)
   //         {                
			//	if((gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] >= 5)&&
   //                (MM_check_packet_head(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index], 
   //                                      gl_modbus_param.slave_addr[gl_NTAGTX_param.vl_BMS_index], 
   //                                      gl_modbus_param.MM_fn_code[gl_NTAGTX_param.vl_BMS_index]))&&
   //                (NTAG_rcv_CRC_check((unsigned char *)(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index]), 
   //                                         gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index])))
   //             {
   //                 #ifdef PMS_IIC_ERROR_COUNT_ENABLE
   //                 //顺便在这里统计读电池成功的次数
   //                 gl_NTAG_read_sucess_cnt++;
   //                 #endif
			//		//数据包正确，跳转处理状态
			//		if(MM_rsp_cmd_process(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index],
			//					gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index],
			//					gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index],
			//					gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]))
			//		{
			//			NTAG_task_timer_reload();
			//			//正确，进行处理
			//			//清空接收缓存
			//			gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//			//标记发送命令结束
			//			gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->waiting_cmd_ack = 0;
			//			//清空缓存数据
			//			gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//			gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//		}
			//		else
			//		{
			//			//错误，进行处理
			//			MM_err_process(gl_modbus_param.MM_NFC_rcv_buff[gl_NTAGTX_param.vl_BMS_index],
			//							gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index],
			//							gl_modbus_param.MM_NFC_snd_buff[gl_NTAGTX_param.vl_BMS_index],
			//							gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index]);
			//			//进行定时器超时流程
			//			//清空接收缓存
			//			gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//			//标记发送命令结束
			//			gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->waiting_cmd_ack = 0;
			//			//清空缓存数据
			//			gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//			gl_modbus_param.MM_NFC_snd_len[gl_NTAGTX_param.vl_BMS_index] = 0;
			//		}
   //             }
   //             else
   //             {
   //                 result = ERROR;
   //             }		
   //         }

   //         if (result==OK)
   //         {             
   //             MM_NFC_snd_done_cb();
			//	//填充要发送的数据
			//	if(0 == Battery_send_cmd(gl_NTAGTX_param.vl_BMS_index))
			//	{
   //                 //所有要发送的数据已发送完毕
   //                 gl_bms_info_p[gl_NTAGTX_param.vl_BMS_index]->online = 1;//设置电池nfc在线
			//		//没有数据，退出
			//		NTAG_task_reset();
			//		break;
			//	}
   //         }
   //         else
   //         {
   //             #ifdef PMS_IIC_ERROR_COUNT_ENABLE
   //             //顺便在这里统计读电池失败次数
   //             gl_NTAG_read_faild_cnt++;
   //             #endif
   //             gl_modbus_param.MM_NFC_rcv_len[gl_NTAGTX_param.vl_BMS_index] = 0;
   //             if(0 == MM_NFC_snd_ERR())
   //             {
   //                 //继续重发
   //                 continue;
   //             }
   //             //进入低功耗
   //             FM175XX_SoftPowerdown();
   //             //重新初始化读卡器任务
   //             NTAG_task_reset();
   //             return ERROR;
   //         }
   //     }
    }
#endif
    return OK;
}

unsigned char NTAG_task_reset(void)
{
    gl_modbus_param.MM_NFC_nead_reset[gl_NTAGTX_param.vl_BMS_index] = 1;
    gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_NTAG_STATE_INIT;
    return 1;
}

void NTAG_task_process(void)
{
    static unsigned char vl_one_time = 0;
    if(!vl_one_time)
    {
        unsigned char i;
        
        vl_one_time = 1;
        for(i = 0; i < NFC_READER_COUNT_MAX; i++)
        {
//            gl_modbus_param.mm_state[i] = ENUM_MM_NFC_INIT;
            gl_modbus_param.MM_NFC_snd_len[i] = 0;//发送缓存中存在的数据长度
            gl_modbus_param.MM_NFC_rcv_len[i] = 0;//接收缓存中存在的数据长度
            gl_modbus_param.MM_NFC_snd_err_cnt[i] = 0;//发送错误计数
            gl_modbus_param.MM_NFC_nead_reset[i] = 0; //等于1表示NFC读卡器需要重启
//            gl_modbus_param.MM_IDLE_timer_cnt = 0;
            gl_NTAG_task_state[i] = ENUM_NTAG_STATE_INIT;
            gl_ntag_found_nfc_cnt[i] = 0;
        }
        MM_nfc_cmd_init();
////		NTAG_task_timer_reload();
    }

    if(gl_modbus_param.MM_NFC_nead_reset[gl_NTAGTX_param.vl_BMS_index])
    {
		if(gl_NTAGTX_param.vl_BMS_index == (NFC_READER_COUNT_MAX-1))
		{
		    rt_sem_release(nfc_sem);
		}
		gl_modbus_param.MM_NFC_nead_reset[gl_NTAGTX_param.vl_BMS_index] = 0;
		FM175XX_HardPowerdown();
        MM_init(gl_NTAGTX_param.vl_BMS_index);
        gl_NTAGTX_param.vl_BMS_index++;
        gl_NTAGTX_param.vl_BMS_index %= NFC_READER_COUNT_MAX;
		gl_NTAG_task_state[gl_NTAGTX_param.vl_BMS_index] = ENUM_NTAG_STATE_INIT;
////		NTAG_task_timer_reload();
    }
    NTAG_task_x();
}

////得到是否在发射NFC读取数据的标志
//unsigned char NTAG_get_nfc_working_status(void)
//{
//    return gl_ntag_nfc_sending_receiving_flag;
//}

