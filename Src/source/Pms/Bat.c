#include "Common.h"
#include "Bat.h"
#include "Battery.h"

#define BMS_STATE_BIT_CHG_SWITCH (1<<0)
#define BMS_STATE_BIT_SUPPLY_SWITCH (1<<1)


//#define		BAT_DEBUG_MSG(fmt,...)		Printf(fmt,##__VA_ARGS__)

#define		BAT_DEBUG_MSG(fmt,...)




//���������
static void Pms_calcChgCur(Bat* pBats, int batCount, uint32 totalPwr)
{

}

unsigned char slave_rs485_is_bat_valid(unsigned char bms_index)
{	
	if(bms_index <= 1)
	{
		Battery* pPkt = Null;
		
		pPkt = &g_Bat[bms_index];
		if(pPkt->presentStatus == BAT_IN)
			return TRUE;
	}
	return FALSE;	
}

//����Ƿ��صĵ�ѹ�͵����Ĵ�����Ч
bool is_battery_A_V_reg_valid(unsigned char bms_index)
{
    Battery* pBat = Null;
	unsigned short vl_tmp;
	
	pBat = &g_Bat[bms_index];

    vl_tmp = bigendian16_get((uint8*)(&pBat->bmsInfo.tvolt));
    if(vl_tmp == 0xFFFF)
        return FALSE;
    
    vl_tmp = bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr));
    if(vl_tmp == 0xFFFF)
        return FALSE;

    return TRUE;
}

//���ǲ����ٴ򿪳�ŵ��
unsigned char is_battery_error(unsigned char bms_index)
{
    unsigned short vl_value;
	Battery* pBat = Null;
	
	pBat = &g_Bat[bms_index];
	
    vl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.devft1));
    if(vl_value&(~((1<<15)|(1<<14)|(1<<0)|(1<<3)|(1<<6)|(1<<8)|(1<<10))))
    {
		BAT_DEBUG_MSG("Bat[%d] devft1 :%04X\n",bms_index,vl_value);
        return 1;
    }

    vl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.devft2));
    if(vl_value&(~(1<<2)))
    {
		BAT_DEBUG_MSG("Bat[%d] devft2 :%04X\n",bms_index,vl_value);
        return 1;
    }

    vl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.opft1));
    if(vl_value&(~((1<<7)|(1<<8))))
    {
		BAT_DEBUG_MSG("Bat[%d] opft1 :%04X\n",bms_index,vl_value);
        return 1;
    }

    vl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.opft2));
    if(vl_value)
    {
		BAT_DEBUG_MSG("Bat[%d] opft2 :%04X\n",bms_index,vl_value);
        return 1;
    }
    
    return 0;
}

unsigned short Battery_get_voltage(unsigned char bms_index)
{
    unsigned int vl_V;
    int vl_A;
	Battery* pBat = Null;
	
	pBat = &g_Bat[bms_index];
	
    vl_V = bigendian16_get((uint8*)(&pBat->bmsInfo.tvolt));
    vl_A = bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr));

    vl_A -= (300*100);

    //���ڵ�ش������30��ŷ���ܹ�16�ڵ��
    //vl_V = vl_V-((30*16)/10) *(vl_A)/100;
    vl_V = vl_V-(150/10) *(vl_A)/100;
    
    return vl_V;
}

// 10mAΪ��λ
int Battery_get_current(unsigned char bms_index)
{
    int vl_A;
	Battery* pBat = Null;
	
	pBat = &g_Bat[bms_index];
    vl_A = bigendian16_get((uint8*)(&pBat->bmsInfo.tcurr));
    vl_A -= (300*100);
    
    return vl_A;
}

unsigned short Battery_get_switch_state(unsigned char bms_index)
{
    //��ŵ綼��ʱ������
    unsigned short vl_ctrl_value;
    Battery* pBat = Null;
	
	pBat = &g_Bat[bms_index];
    vl_ctrl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.state));

    return vl_ctrl_value;
}

#define BATTERY_VOLTAGE_SHAKE_CNT_MAX 600
static uint32_t gl_Battery_voltage_shake_cnt;
static SwTimer gl_Battery_optime_timeout = { 0 };
// ˫��طŵ��㷨
void Battery_discharge_process(void)
{
    unsigned short vl_bms_0_ctrl, vl_bms_1_ctrl;
	unsigned short vl_bms_0_V, vl_bms_1_V;
	int vl_bms_0_A, vl_bms_1_A;
	unsigned short vl_voltage_diff;
    int vl_current_diff;
    static unsigned char sl_supply_state = 0; // 1, ��ʾ��ѹ��С��0.5v��2,��ʾ��ѹ�����0.5v
    static unsigned char sl_bms0_vol_cmp_offset = 0; 
    static unsigned char sl_bms1_vol_cmp_offset = 0;
	
//    unsigned char vl_force_to_exe[2] = {0, 0};

//    unsigned short vl_pos, vl_state_pos;


    //���Դ�ӡ��ѹ��ȱ�־
    static unsigned char sl_both_voltage_equ_flag = 0;
    static unsigned char sl_both_voltage_diff_flag = 0;

    if(((slave_rs485_is_bat_valid(0))&&(is_battery_A_V_reg_valid(0))&&(0 == is_battery_error(0)))&&
	   ((slave_rs485_is_bat_valid(1))&&(is_battery_A_V_reg_valid(1))&&(0 == is_battery_error(1))))
	{
	    //������ض�����
        vl_bms_0_V = Battery_get_voltage(0);
        vl_bms_1_V = Battery_get_voltage(1);

        vl_bms_0_A = Battery_get_current(0);
        vl_bms_1_A = Battery_get_current(1);
	
		vl_bms_0_ctrl = Battery_get_switch_state(0);
        vl_bms_1_ctrl = Battery_get_switch_state(1);

		//PFL(DL_NFC,
		BAT_DEBUG_MSG("Bat0:%d-%d-%X,Bat1:%d-%d-%X |%d\n",
			vl_bms_0_V,vl_bms_0_A,vl_bms_0_ctrl,vl_bms_1_V,vl_bms_1_A,vl_bms_1_ctrl,sl_supply_state);
		
		if(vl_bms_0_V >= vl_bms_1_V)
        {
            vl_voltage_diff = vl_bms_0_V - vl_bms_1_V;
            //��������Ǹ�ֵ������ʱ��ֱ
            vl_current_diff = vl_bms_1_A - vl_bms_0_A;
        }
        else
        {
            vl_voltage_diff = vl_bms_1_V - vl_bms_0_V;
            //��������Ǹ�ֵ������ʱ��ֱ
            vl_current_diff = vl_bms_0_A - vl_bms_1_A;
        }
		BAT_DEBUG_MSG("vol_diff:%d,cur_diff:%d\n",vl_voltage_diff,vl_current_diff);
		
		if(sl_supply_state == 1)//ͬʱ����
        {
            if((vl_voltage_diff < 600)&&(vl_current_diff < 2500) )
            {
                gl_Battery_voltage_shake_cnt = GET_TICKS();
            }
            else if((GET_TICKS() -gl_Battery_voltage_shake_cnt) > BATTERY_VOLTAGE_SHAKE_CNT_MAX )
            {
                // ��ѹ����ڵ���6V���ߵ��������25A����Ϊ��������
                sl_supply_state = 2;
                sl_both_voltage_equ_flag = 0;
            }
        }
        else if(sl_supply_state == 2)//��������
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
                gl_Battery_voltage_shake_cnt = GET_TICKS();
            }
            else if((GET_TICKS() -gl_Battery_voltage_shake_cnt) > (BATTERY_VOLTAGE_SHAKE_CNT_MAX*2) )//����ȥ��ʱ��
            {
                //������ض��е��������ҵ�����С��20A����Ϊͬʱ����
                sl_supply_state = 1;
            }
        }
        else
        {
            gl_Battery_voltage_shake_cnt = GET_TICKS();
            sl_supply_state = 2;
			sl_both_voltage_equ_flag = 0;
			sl_both_voltage_diff_flag = 0;
        }
		//������ص�ѹ����
        if(1 == sl_supply_state)
        {
            //�򿪵��0�ĳ�ŵ翪��
			if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
			{
				Bat_setDischg(&g_Bat[0], True);
				Bat_setChg(&g_Bat[0], True);
				BAT_DEBUG_MSG("Bat[0] Start DisChg&Chg\n");
			}
			//�򿪵��1�ĳ�ŵ翪��
			if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 0)||
					((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 0))
			{
				Bat_setDischg(&g_Bat[1], True);
				Bat_setChg(&g_Bat[1], True);
				BAT_DEBUG_MSG("Bat[1] Start DisChg&Chg\n");
			}
            sl_bms0_vol_cmp_offset = 0;
            sl_bms1_vol_cmp_offset = 0;
        }
        else if(2 == sl_supply_state)//����ؽ���
        {             
            //�رյ͵�ѹ�ĵ�س�ŵ翪��
            if((vl_bms_0_V - sl_bms0_vol_cmp_offset)> (vl_bms_1_V - sl_bms1_vol_cmp_offset))
            {
                sl_bms0_vol_cmp_offset = 0;
                sl_bms1_vol_cmp_offset = 100;


                if(vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH)
                {
                    //�͵�ѹ��صĳ�翪���ǵ�ͨ�ģ���ʱҪ��ر�
                    Bat_setChg(&g_Bat[1], False);
					BAT_DEBUG_MSG("Bat[1] Stop Chg\n");
                }
                if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //�ߵ�ѹ��صķŵ翪���ǹرյģ���ʱҪ���
                    Bat_setDischg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start DisChg\n");
                }                
                if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //�͵�ѹ��صķŵ翪���ǹرյģ���ʱҪ���
                   Bat_setDischg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start DisChg\n");
                }                
                if((vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
                {
                    //�ߵ�ѹ��صĳ�翪���ǹرյģ���ʱҪ���
                    Bat_setChg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start Chg\n");
                }
			}
			else
			{            
				sl_bms0_vol_cmp_offset = 100;
				sl_bms1_vol_cmp_offset = 0;

				if(vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH)
				{
					//�͵�ѹ��صĳ�翪���ǵ�ͨ�ģ���ʱҪ��ر�
					Bat_setChg(&g_Bat[0], False); 
					BAT_DEBUG_MSG("Bat[0] Stop Chg\n");
				}
				if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
				{
					//�ߵ�ѹ��صķŵ翪���ǹرյģ���ʱҪ���
					Bat_setDischg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start DisChg\n");
				}
				if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
				{
					//�͵�ѹ��صķŵ翪���ǹرյģ���ʱҪ���
					Bat_setDischg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start DisChg\n");
				}
				if((vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
				{
					//�ߵ�ѹ��صĳ�翪���ǹرյģ���ʱҪ���
					Bat_setChg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start Chg\n");
				}
			}
		}
	}
	else if((slave_rs485_is_bat_valid(0))&&(is_battery_A_V_reg_valid(0))&&
	        (0 == is_battery_error(0)))
    {
		//���0����
//        vl_bms_0_A = Battery_get_current(0);

        vl_bms_0_ctrl=Battery_get_switch_state(0);

        //�򿪵��0�ĳ�ŵ翪��
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
		{
			Bat_setDischg(&g_Bat[0], True);
			Bat_setChg(&g_Bat[0], True);
			BAT_DEBUG_MSG("Bat[0] Start DisChg&Chg\n");
        }
    }
	else if((slave_rs485_is_bat_valid(1))&&(is_battery_A_V_reg_valid(1))&&
	       (0 == is_battery_error(1)))
    {
		//���1����
//        vl_bms_1_A = Battery_get_current(1);

        vl_bms_1_ctrl=Battery_get_switch_state(1);

        //�򿪵��0�ĳ�ŵ翪��
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 0))
		{
			Bat_setDischg(&g_Bat[1], True);
			Bat_setChg(&g_Bat[1], True);
			BAT_DEBUG_MSG("Bat[1] Start DisChg&Chg\n");
        }
    }
}



