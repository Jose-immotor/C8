/*
 * File      : battery_process.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-10     lane      first implementation
 */

#include <string.h>
#include "pms.h"
#include "modbus_master.h"
#include "battery.h"
#include <stdint.h>

unsigned char gl_cmd_buf[NFC_READER_COUNT_MAX][ENUM_NFC_CMD_INDEX_MAX];

#define BATTERY_VOLTAGE_SHAKE_CNT_MAX 600
static unsigned int gl_Battery_voltage_shake_cnt;


void Battery_timer_count_cb(void)
{
	if(gl_Battery_voltage_shake_cnt)
        gl_Battery_voltage_shake_cnt--;
}
////static unsigned char sl_battery_need2sleep_flag = 0;// 1表示让电池进入休眠状态
unsigned char gl_illegal_charging_in_flag = 0;

void Battery_set_reg_valid_flag(unsigned char bms_index, unsigned char flag)
{
    gl_bms_info_p[bms_index]->reg_value_ready |= flag;    
}

unsigned short Battery_get_reg_value(unsigned char bms_index, unsigned short reg_index)
{
    unsigned short vl_pos;
    vl_pos = MM_get_reg_addr_position_in_table(reg_index);

    vl_pos = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    return vl_pos;
}

unsigned short Battery_get_voltage(unsigned char bms_index)
{
    unsigned short vl_pos;
    unsigned int vl_V;
    int vl_A;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TVOLT);

    vl_V = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR);
    vl_A = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    vl_A -= (300*100);

    //单节电池大概内阻30毫欧，总共16节电池
    //vl_V = vl_V-((30*16)/10) *(vl_A)/100;
    vl_V = vl_V-(150/10) *(vl_A)/100;
    
    return vl_V;
}

//返回 0.1%精度
unsigned short Battery_get_SOC(unsigned char bms_index)
{
    unsigned short vl_pos;
    unsigned short vl_SOC;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_SOC);

    vl_SOC = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    if(vl_SOC == 0xFFFF)
        vl_SOC = 0;
    
    return vl_SOC;
}

void Battery_clear_reg_valid_flag(unsigned char bms_index)
{
    gl_bms_info_p[bms_index]->reg_value_ready = 0;
}

void Battery_cmd_buf_clear(unsigned char bms_index)
{
    memset(&gl_cmd_buf[bms_index][0], 0, ENUM_NFC_CMD_INDEX_MAX);
}

//单个NFC读卡器是否所有命令已经发送完成
bool Battery_is_one_cmd_buf_empty(unsigned char bms_index)
{
    unsigned char i;

    for(i = 0; i < (ENUM_NFC_CMD_INDEX_MAX); i++)
    {
        if(gl_cmd_buf[bms_index][i])
        {
            return FALSE;
        }
    }
    return TRUE;
}
unsigned char Battery_send_cmd(unsigned char bms_index)
{
    unsigned char i;
    i = bms_index;
    //for(i = 0; i < NFC_READER_COUNT_MAX; i++)
    {
        if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_W_RESET])
        {
            unsigned short vl_k;
            unsigned short vl_cnt;
            st_write_reg_10 vl_write_reg_10;
            
            vl_cnt = 1; //CTRL_REG_TOTAL_SIZE
            vl_write_reg_10.reg_addr_begin[0]=(ENUM_REG_ADDR_RESET>>8)&0xFF;
            vl_write_reg_10.reg_addr_begin[1]=(ENUM_REG_ADDR_RESET)&0xFF;
            vl_write_reg_10.reg_cnt[0] = (vl_cnt>>8)&0xFF;
            vl_write_reg_10.reg_cnt[1] = (vl_cnt)&0xFF;
            vl_write_reg_10.byte_cnt = vl_cnt*2;

            vl_k = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_RESET);
            
            //设置重启值
            gl_bms_info_p[i]->reg_unit[vl_k].bits16_H = 0x7A;
            gl_bms_info_p[i]->reg_unit[vl_k].bits16_L= 0xB9;
            
            vl_write_reg_10.byte_data = (unsigned char*)&gl_bms_info_p[i]->reg_unit[vl_k];
            //与电池通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_write_cmd(MM_ADDR_VALUE, &vl_write_reg_10))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_W_RESET] = 0;
                    return 1;
                }
            }
        }
    }
    
    //for(i = 0; i < NFC_READER_COUNT_MAX; i++)
    {
        if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_W_CTRL])
        {
            unsigned short vl_k;
            unsigned short vl_cnt;
            st_write_reg_10 vl_write_reg_10;
          
            vl_cnt = 1; //CTRL_REG_TOTAL_SIZE
            vl_write_reg_10.reg_addr_begin[0]=(CTRL_REG_BIGIN_ADDR>>8)&0xFF;
            vl_write_reg_10.reg_addr_begin[1]=(CTRL_REG_BIGIN_ADDR)&0xFF;
            vl_write_reg_10.reg_cnt[0] = (vl_cnt>>8)&0xFF;
            vl_write_reg_10.reg_cnt[1] = (vl_cnt)&0xFF;
            vl_write_reg_10.byte_cnt = vl_cnt*2;

            vl_k = MM_get_reg_addr_position_in_table(CTRL_REG_BIGIN_ADDR);
            vl_write_reg_10.byte_data = (unsigned char*)&gl_bms_info_p[i]->reg_unit[vl_k];
            //与电池通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_write_cmd(MM_ADDR_VALUE, &vl_write_reg_10))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_W_CTRL] = 0;
                    return 1;
                }
            }
        }
    }

    //for(i = 0; i < NFC_READER_COUNT_MAX; i++)
    {
        if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_VERSION])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=(DEV_INFO_REG_BIGIN_ADDR>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=(DEV_INFO_REG_BIGIN_ADDR)&0xFF;
            vl_read_reg_03.reg_cnt[0] = (DEV_INFO_REG_TOTAL_SIZE>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = (DEV_INFO_REG_TOTAL_SIZE)&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_VERSION] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_INFO1])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=(READ_ONLY_REG_BIGIN_ADDR>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=(READ_ONLY_REG_BIGIN_ADDR)&0xFF;
            vl_read_reg_03.reg_cnt[0] = ((READ_ONLY_REG_TOTAL_SIZE/2)>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = ((READ_ONLY_REG_TOTAL_SIZE/2))&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_INFO1] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_INFO2])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=((READ_ONLY_REG_BIGIN_ADDR+(READ_ONLY_REG_TOTAL_SIZE/2))>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=((READ_ONLY_REG_BIGIN_ADDR+(READ_ONLY_REG_TOTAL_SIZE/2)))&0xFF;
            //判断电池版本号是否大于等于35版本
            if(((BMS_DEV_INFO_FLAG_BIT)&Battery_get_reg_valid_flag(bms_index))&&
                (35 <= Battery_get_reg_value(bms_index, ENUM_REG_ADDR_FWREV)))
            {
                //支持寄存器地址322
                vl_read_reg_03.reg_cnt[0] = ((READ_ONLY_REG_TOTAL_SIZE-(READ_ONLY_REG_TOTAL_SIZE/2))>>8)&0xFF;
                vl_read_reg_03.reg_cnt[1] = ((READ_ONLY_REG_TOTAL_SIZE-(READ_ONLY_REG_TOTAL_SIZE/2)))&0xFF;
            }
            else
            {
                vl_read_reg_03.reg_cnt[0] = ((READ_ONLY_REG_TOTAL_SIZE-(READ_ONLY_REG_TOTAL_SIZE/2) - 7)>>8)&0xFF;
                vl_read_reg_03.reg_cnt[1] = ((READ_ONLY_REG_TOTAL_SIZE-(READ_ONLY_REG_TOTAL_SIZE/2) - 7))&0xFF;
            }

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_INFO2] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_CTRL])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=(CTRL_REG_BIGIN_ADDR>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=(CTRL_REG_BIGIN_ADDR)&0xFF;
            vl_read_reg_03.reg_cnt[0] = (CTRL_REG_TOTAL_SIZE>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = (CTRL_REG_TOTAL_SIZE)&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_CTRL] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_PARAM1])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=(PARAM_REG_BIGIN_ADDR>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=(PARAM_REG_BIGIN_ADDR)&0xFF;
            vl_read_reg_03.reg_cnt[0] = ((PARAM_REG_TOTAL_SIZE/2)>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = ((PARAM_REG_TOTAL_SIZE/2))&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_PARAM1] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_PARAM2])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=((PARAM_REG_BIGIN_ADDR+(PARAM_REG_TOTAL_SIZE/2))>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=((PARAM_REG_BIGIN_ADDR+(PARAM_REG_TOTAL_SIZE/2)))&0xFF;
            vl_read_reg_03.reg_cnt[0] = ((PARAM_REG_TOTAL_SIZE-(PARAM_REG_TOTAL_SIZE/2))>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = ((PARAM_REG_TOTAL_SIZE-(PARAM_REG_TOTAL_SIZE/2)))&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_PARAM2] = 0;
                    return 1;
                }
            }
        }
        else if(gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_USERDATA])
        {
            st_read_reg_03 vl_read_reg_03;
            vl_read_reg_03.reg_addr_begin[0]=(USER_REG_BIGIN_ADDR>>8)&0xFF;
            vl_read_reg_03.reg_addr_begin[1]=(USER_REG_BIGIN_ADDR)&0xFF;
            vl_read_reg_03.reg_cnt[0] = (USER_REG_TOTAL_SIZE>>8)&0xFF;
            vl_read_reg_03.reg_cnt[1] = (USER_REG_TOTAL_SIZE)&0xFF;

            //与电池1通讯
            //if(MM_set_BMS_index(i))
            {
                if(MM_snd_multi_read_cmd(MM_ADDR_VALUE, &vl_read_reg_03))
                {
                    gl_cmd_buf[i][ENUM_NFC_CMD_INDEX_R_USERDATA] = 0;
                    return 1;
                }
            }
        }     
    }    

    return 0;
}

unsigned char Battery_get_reg_valid_flag(unsigned char bms_index)
{
    return gl_bms_info_p[bms_index]->reg_value_ready;
}

//检查是否要断电
unsigned char main_is_cmd_poweroff_valid_immediately(void)
{
#if 0
    if(g_pPms->m_PowerOffAtOnce)
    {
        return 1;
    }
#endif
    return 0;
}
unsigned short Battery_get_switch_state(unsigned char bms_index)
{
    //充放电都打开时候才算打开
    unsigned short vl_pos, vl_ctrl_value;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_STATE);
    vl_ctrl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    return vl_ctrl_value;
}

unsigned char slave_rs485_is_bat_valid(unsigned char bms_index)
{
//	if((gl_hall_gpio_state&(1<<bms_index))&&(gl_bms_info_p[bms_index]->online))
	//暂时去掉，霍尔电路不能用
	if(gl_bms_info_p[bms_index]->online)
	{
		return TRUE;
	}
	return FALSE;
}

// 10mA为单位
int Battery_get_current(unsigned char bms_index)
{
    unsigned short vl_pos;
    int vl_A;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR);
    vl_A = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    vl_A -= (300*100);
    
    return vl_A;
}
//就是不能再打开充放电管
unsigned char is_battery_error(unsigned char bms_index)
{
    unsigned short vl_pos, vl_value;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT1);
    vl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_value&(~((1<<15)|(1<<14)|(1<<0)|(1<<3)|(1<<6)|(1<<8)|(1<<10))))
    {
        return 1;
    }

    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT2);
    vl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_value&(~(1<<2)))
    {
        return 1;
    }

    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT1);
    vl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_value&(~((1<<7)|(1<<8))))
    {
        return 1;
    }

    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_DEVFT2);
    vl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_value)
    {
        return 1;
    }
    
    return 0;
}
//检测是否电池的电压和电流寄存器有效
bool is_battery_A_V_reg_valid(unsigned char bms_index)
{
    
    unsigned short vl_pos;
    //unsigned int vl_V;
    //int vl_A;
    unsigned short vl_tmp;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TVOLT);

    vl_tmp = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_tmp == 0xFFFF)
        return FALSE;
    
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_TCURR);
    vl_tmp = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_tmp == 0xFFFF)
        return FALSE;

    return TRUE;
}
//是否预放失败，需要清零充放电寄存器，这样后面才能打开充放电管
unsigned char is_battery_presupply_error(unsigned char bms_index)
{
    unsigned short vl_pos, vl_value;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_OPFT1);
    vl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);
    if(vl_value&(1<<15))
    {
        return 1;
    }

    return 0;
}
void Battery_clear_switch_state(unsigned char bms_index, unsigned short on)
{
    unsigned short vl_pos, vl_ctrl_value;
    
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CTRL);
    vl_ctrl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
        gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    vl_ctrl_value &=~(on);
        
    gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H = (vl_ctrl_value>>8)&0xff;
    gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L = (vl_ctrl_value>>0)&0xff;
    
    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_W_CTRL] = 1;   
}
void Battery_set_switch_state(unsigned char bms_index, unsigned short on)
{
    unsigned short vl_pos, vl_ctrl_value;
  
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CTRL);
    vl_ctrl_value = ((gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H<<8) + 
                      gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L);

    vl_ctrl_value |= on;

    gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_H = (vl_ctrl_value>>8)&0xff;
    gl_bms_info_p[bms_index]->reg_unit[vl_pos].bits16_L = (vl_ctrl_value>>0)&0xff;

    gl_cmd_buf[bms_index][ENUM_NFC_CMD_INDEX_W_CTRL] = 1;    
}
//检查是否有预放错误，然后再设置
void Bttery_check_pre_error_and_set_switch_state(unsigned char bms_index, unsigned short on)
{
    unsigned short vl_pos;
    vl_pos = MM_get_reg_addr_position_in_table(ENUM_REG_ADDR_CTRL);
    if(is_battery_presupply_error(0)&&//是否有预放电错误
        (gl_bms_info_p[0]->reg_unit[vl_pos].bits16_L&0x03))//控制寄存器上的充放电位不为零
    {
        //需要清零充放电控制寄存器
        Battery_clear_switch_state(bms_index,0xFFFF);
        //CmdLine_Printf("pre error, clear ctrl!\r\n");
    }
    else
    {
        Battery_set_switch_state(bms_index, on);
    }
}
void Battery_sleep_process(void)
{
    unsigned short vl_bms_0_ctrl, vl_bms_1_ctrl;    
    
    //判断电池是否都在线
    if((slave_rs485_is_bat_valid(0))&& (slave_rs485_is_bat_valid(1)))
    {
        //两个电池都在线
        vl_bms_0_ctrl = Battery_get_switch_state(0);
        vl_bms_1_ctrl = Battery_get_switch_state(1);

        if(vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH)
        {
            //关闭充电开关
            Battery_clear_switch_state(0, BMS_STATE_BIT_CHG_SWITCH);
        }
        else if(vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH)
        {
            //关闭充电开关
            Battery_clear_switch_state(1, BMS_STATE_BIT_CHG_SWITCH);
        }
        else if(vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH)
        {
            //关闭放电开关
            Battery_clear_switch_state(0, BMS_STATE_BIT_SUPPLY_SWITCH);
        }
        else if(vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH)
        {
            //关闭放电开关
            Battery_clear_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
        }
        else if(vl_bms_0_ctrl & BMS_STATE_BIT_PRESUPPLY_SWITCH)
        {
            //关闭预放电开关
            Battery_clear_switch_state(0, BMS_STATE_BIT_PRESUPPLY_SWITCH);
        }
        else if(vl_bms_1_ctrl & BMS_STATE_BIT_PRESUPPLY_SWITCH)
        {
            //关闭预放电开关
            Battery_clear_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
        }

    }
    else if(slave_rs485_is_bat_valid(0)) 
    {
        //电池0在线
        vl_bms_0_ctrl = Battery_get_switch_state(0);

        //打开电池0的充放电开关
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 1)||((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 1))
        {
            Battery_clear_switch_state(0, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH|BMS_STATE_BIT_PRESUPPLY_SWITCH);
        }
        
    }
    else if(slave_rs485_is_bat_valid(1))
    {
        //电池1在线
        vl_bms_1_ctrl = Battery_get_switch_state(1);

        //打开电池1的充放电开关
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 1)||((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 1))
        {
            Battery_clear_switch_state(1, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH|BMS_STATE_BIT_PRESUPPLY_SWITCH);
        }
    }
    else
    {
        //电池故障
        
    }
}

void Battery_discharge_process(void)
{
    unsigned short vl_bms_0_ctrl, vl_bms_1_ctrl;
	unsigned short vl_bms_0_V, vl_bms_1_V;
	int vl_bms_0_A, vl_bms_1_A;
	unsigned short vl_voltage_diff;
    int vl_current_diff;
    static unsigned char sl_supply_state = 0; // 1, 表示电压差小于0.5v；2,表示电压差大于0.5v
    static unsigned char sl_bms0_vol_cmp_offset = 0; 
    static unsigned char sl_bms1_vol_cmp_offset = 0;
	
	
//    unsigned char vl_force_to_exe[2] = {0, 0};

//    unsigned short vl_pos, vl_state_pos;

    //每秒查询一次电流，确认是否充电器插入
//    static unsigned int sl_old_tick_cnt = 0;
//    unsigned char vl_check_charger_current_in_flag_1_sec = 0; // 1秒钟标志

    //测试打印电压相等标志
    static unsigned char sl_both_voltage_equ_flag = 0;
    static unsigned char sl_both_voltage_diff_flag = 0;

    if(main_is_cmd_poweroff_valid_immediately())//立即执行
    {
        Battery_sleep_process();
        return ;
    }


    if(((slave_rs485_is_bat_valid(0))&&(is_battery_A_V_reg_valid(0))&&(0 == is_battery_error(0)))&&
	   ((slave_rs485_is_bat_valid(1))&&(is_battery_A_V_reg_valid(1))&&(0 == is_battery_error(1))))
	{
	    //两个电池都在线
        vl_bms_0_V = Battery_get_voltage(0);
        vl_bms_1_V = Battery_get_voltage(1);

        vl_bms_0_A = Battery_get_current(0);
        vl_bms_1_A = Battery_get_current(1);

        //如果再有输出电流1A以上，则复位非法充电标志位
        if((vl_bms_0_A+vl_bms_1_A) < (-100))
        {
            gl_illegal_charging_in_flag = 0;
        }		
		vl_bms_0_ctrl = Battery_get_switch_state(0);
        vl_bms_1_ctrl = Battery_get_switch_state(1);
		if(vl_bms_0_V >= vl_bms_1_V)
        {
            vl_voltage_diff = vl_bms_0_V - vl_bms_1_V;
            //输出电流是负值，输入时正直
            vl_current_diff = vl_bms_1_A - vl_bms_0_A;
        }
        else
        {
            vl_voltage_diff = vl_bms_1_V - vl_bms_0_V;
            //输出电流是负值，输入时正直
            vl_current_diff = vl_bms_0_A - vl_bms_1_A;
        }
		if(sl_supply_state == 1)//同时接入
        {
            if((vl_voltage_diff < 600)&&(vl_current_diff < 2500) )
            {
                gl_Battery_voltage_shake_cnt = BATTERY_VOLTAGE_SHAKE_CNT_MAX;
            }
            else if(!gl_Battery_voltage_shake_cnt)
            {
                // 电压差大于等于6V或者电流差大于25A，变为单个接入
                sl_supply_state = 2;
                rt_kprintf("Changed to one Bat!\r\n");
                sl_both_voltage_equ_flag = 0;
            }
        }
        else if(sl_supply_state == 2)//单个接入
        {
            unsigned char vl_current_flag = 0;
            if((vl_bms_0_A > 20)||(vl_bms_0_A < -20))
            {
                vl_current_flag |= 0x01;
            }
            if((vl_bms_1_A > 20)||(vl_bms_1_A < -20))
            {
                vl_current_flag |= 0x02;
            }            
            if((vl_current_diff >= 2000)||(vl_current_flag != 0x03))
            {
                gl_Battery_voltage_shake_cnt = (BATTERY_VOLTAGE_SHAKE_CNT_MAX+BATTERY_VOLTAGE_SHAKE_CNT_MAX);//两倍去抖时间
            }
            else if(!gl_Battery_voltage_shake_cnt)
            {
                //两个电池都有电流，并且电流差小于20A，变为同时接入
                sl_supply_state = 1;
                rt_kprintf("Changed to two Bat!\r\n");
            }
        }
        else
        {
            gl_Battery_voltage_shake_cnt = BATTERY_VOLTAGE_SHAKE_CNT_MAX;
            sl_supply_state = 2;
			sl_both_voltage_equ_flag = 0;
			sl_both_voltage_diff_flag = 0;

        }
		//两个电池电压接入
        if(1 == sl_supply_state)
        {
            //两电池电压差小于0.5v
//            Battery_clear_high_voltage_bat_err_param(2);

            // 一秒调用一次，检测是否是充电器插入在充电
//            if((vl_check_charger_current_in_flag_1_sec)&&
//                (1 == Battery_check_two_chargers_current_in()))
//            {
//                DEBUG_PRINTF("Illegal charger in. 2\r\n");
//                gl_illegal_charging_in_flag = 1;
//                
//                vl_check_charger_current_in_flag_1_sec = 0;

//            }
            
//            if(sl_both_voltage_equ_flag == 0)
//            {
//                sl_both_voltage_equ_flag = 1;
//                DEBUG_PRINTF("Both voltage, small different!\r\n");
//            }
//            sl_both_voltage_diff_flag = 0;

            
            //比较两电池的BMS控制寄存器的开关状态是否符合要求
            //同时打开两电池的充放电开关
			if((gl_illegal_charging_in_flag)&&(BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl))
            {
                Battery_clear_switch_state(0, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
            }
            else if((gl_illegal_charging_in_flag)&&(BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl))
            {
                Battery_clear_switch_state(1, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
            }
            else if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                    ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
            {
                if(gl_illegal_charging_in_flag)
                {
                    Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_SUPPLY_SWITCH);
					Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
                }
                else
                {
                    Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH);
                    Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH);
               }

            }
            sl_bms0_vol_cmp_offset = 0;
            sl_bms1_vol_cmp_offset = 0;
        }
        else if(2 == sl_supply_state)//单电池接入
        {             
            //两电池压差大于0.5V
//            if(sl_both_voltage_diff_flag == 0)
//            {
//                sl_both_voltage_diff_flag = 1;
//                DEBUG_PRINTF("Both voltage, big different!\r\n");
//            }
//            sl_both_voltage_equ_flag = 0;

//            Battery_check_high_voltage_bat_err(0);
//            Battery_check_high_voltage_bat_err(1);
            
            //比较两电池的BMS控制寄存器的开关状态是否符合要求

            //关闭低电压的电池充放电开关
            if((vl_bms_0_V - sl_bms0_vol_cmp_offset)> (vl_bms_1_V - sl_bms1_vol_cmp_offset))
            {
                sl_bms0_vol_cmp_offset = 0;
                sl_bms1_vol_cmp_offset = 100;

                // 一秒调用一次，检测是否是充电器插入在充电
//                if((vl_check_charger_current_in_flag_1_sec)&&
//                    (Battery_check_charger_current_in(0)))
//                {
//                    DEBUG_PRINTF("Illegal charger in. 2.0\r\n");
//                    gl_illegal_charging_in_flag = 1;
//                    
//                    vl_check_charger_current_in_flag_1_sec = 0;
//                    //两个电池都是充电器插入的充电电流状态
//                    // 设置电池不允许放电
//                    //Battery_set_PMS_working_state(0);
//                    //Battery_set_battery_sleep();
//                    //masked, 2018-6-30, 检测到充电只关充电管。
//                    //Battery_set_battery_sleep_by_delay(ENUM_BAT_SLEEP_SRC_ILLEGAL_CHARGER_2_0, 5000);
//                }
                
                //DEBUG_PRINTF("bms 1 voltage, small!\r\n");
                if((gl_illegal_charging_in_flag)&&(BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl))
                {
                    Battery_clear_switch_state(0, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
                }
                else if((gl_illegal_charging_in_flag)&&(BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl))
                {
                    Battery_clear_switch_state(1, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
                }
                else if(vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH)
                {
                    //低电压电池的充电开关是导通的，这时要求关闭
                    Battery_clear_switch_state(1, BMS_STATE_BIT_CHG_SWITCH);   
                }
                else if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //高电压电池的放电开关是关闭的，这时要求打开
                    Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_SUPPLY_SWITCH);
                }                
                else if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //低电压电池的放电开关是关闭的，这时要求打开
                    Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
                }                
                else if((vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
                {
                    //高电压电池的充电开关是关闭的，这时要求打开
                    Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_CHG_SWITCH);
                }
			}
        }
        else
        {            
			sl_bms0_vol_cmp_offset = 100;
			sl_bms1_vol_cmp_offset = 0;
                
                //DEBUG_PRINTF("bms 0 voltage, small!\r\n");
                // 一秒调用一次，检测是否是充电器插入在充电
//                if((vl_check_charger_current_in_flag_1_sec)&&
//                    (Battery_check_charger_current_in(1)))
//                {
//                    DEBUG_PRINTF("Illegal charger in. 2.1\r\n");
//                    gl_illegal_charging_in_flag = 1;
//                    
//                    vl_check_charger_current_in_flag_1_sec = 0;
                    //两个电池都是充电器插入的充电电流状态
                    // 设置电池不允许放电
                    //Battery_set_PMS_working_state(0);
                    //Battery_set_battery_sleep();
                    //masked, 2018-6-30, 检测到充电只关充电管。
                    //Battery_set_battery_sleep_by_delay(ENUM_BAT_SLEEP_SRC_ILLEGAL_CHARGER_2_1, 5000);
//                }

            if((gl_illegal_charging_in_flag)&&(vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH))
            {
                Battery_clear_switch_state(0, (BMS_STATE_BIT_CHG_SWITCH));
            }
            else if((gl_illegal_charging_in_flag)&&(vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH))
            {
                Battery_clear_switch_state(1, (BMS_STATE_BIT_CHG_SWITCH));
            }
            else if(vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH)
            {
                //低电压电池的充电开关是导通的，这时要求关闭
                Battery_clear_switch_state(0, BMS_STATE_BIT_CHG_SWITCH);
            }
            else if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
            {
                //高电压电池的放电开关是关闭的，这时要求打开
                Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
            }
            else if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
            {
                //低电压电池的放电开关是关闭的，这时要求打开
                Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_SUPPLY_SWITCH);
            }
            else if((vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
            {
                //高电压电池的充电开关是关闭的，这时要求打开
                Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_CHG_SWITCH);
            }
        }
		
	}
	else if((slave_rs485_is_bat_valid(0))&&
	        (is_battery_A_V_reg_valid(0))&&
	        (0 == is_battery_error(0)))
    {
		//电池0可用
        vl_bms_0_A = Battery_get_current(0);
        //如果再有输出电流500mA以上，则复位非法充电标志位
        if((vl_bms_0_A) < (-50))
        {
            gl_illegal_charging_in_flag = 0;
        }
        vl_bms_0_ctrl=Battery_get_switch_state(0);
        if((gl_illegal_charging_in_flag)&&((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl)))
        {
            Battery_clear_switch_state(0, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
        }
        //打开电池0的充放电开关
        else if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
		{
            if(gl_illegal_charging_in_flag)
            {
                Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_SUPPLY_SWITCH);
            }
            else
            {
                Bttery_check_pre_error_and_set_switch_state(0, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH);
            }
        }
    }
	else if((slave_rs485_is_bat_valid(1))&&
	       (is_battery_A_V_reg_valid(1))&&
	       (0 == is_battery_error(1)))
    {
		//电池1可用
        vl_bms_1_A = Battery_get_current(1);
        //如果再有输出电流500mA以上，则复位非法充电标志位
        if((vl_bms_1_A) < (-50))
        {
            gl_illegal_charging_in_flag = 0;
        }
        vl_bms_1_ctrl=Battery_get_switch_state(1);
        if((gl_illegal_charging_in_flag)&&((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl)))
        {
            Battery_clear_switch_state(1, (BMS_STATE_BIT_CHG_SWITCH)); //关闭电池充电管
        }
        //打开电池0的充放电开关
        else if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 0))
		{
            if(gl_illegal_charging_in_flag)
            {
                Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_SUPPLY_SWITCH);
            }
            else
            {
                Bttery_check_pre_error_and_set_switch_state(1, BMS_STATE_BIT_CHG_SWITCH| BMS_STATE_BIT_SUPPLY_SWITCH);
            }
        }
    }
}


void Battery_Process(void)
{
	Battery_discharge_process();
}

void thread_battery_entry(void* parameter)
{  
	rt_err_t result;
	Pms_Init();
    while (1)
    {
        result = rt_sem_take(nfc_sem, RT_WAITING_FOREVER);
		if (result == RT_EOK)
        {
			Battery_Process();
			Pms_Run();
			slave_rs485_cmd_param_changed_polling();
		}
		//延迟
        rt_thread_mdelay(100);
    }            
}

static void battery_ms_timer_cb(void* parameter)
{
	Battery_timer_count_cb();
}

struct rt_thread thread_battery;
unsigned char thread_battery_stack[1024];
rt_timer_t battery_ms_handler;
static int app_bat_init(void)
{
	rt_err_t res;

	res=rt_thread_init(&thread_battery,"battery",thread_battery_entry,RT_NULL,&thread_battery_stack[0],
	sizeof(thread_battery_stack), 4, 10);
    if (res == RT_EOK) /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(&thread_battery);
	else
		rt_kprintf("\r\n!!create thread battery failed!\r\n");

	battery_ms_handler = rt_timer_create("battery_ms_timer", battery_ms_timer_cb, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);
    if (battery_ms_handler != RT_NULL) 
        rt_timer_start(battery_ms_handler);
    return 0;
}
INIT_APP_EXPORT(app_bat_init);
