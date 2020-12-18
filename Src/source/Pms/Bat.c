#include "Common.h"
#include "Bat.h"
#include "Battery.h"

#define BMS_STATE_BIT_CHG_SWITCH (1<<0)
#define BMS_STATE_BIT_SUPPLY_SWITCH (1<<1)


//#define		BAT_DEBUG_MSG(fmt,...)		Printf(fmt,##__VA_ARGS__)

#define		BAT_DEBUG_MSG(fmt,...)




//计算充电电流
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

//检测是否电池的电压和电流寄存器有效
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

//就是不能再打开充放电管
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

    //单节电池大概内阻30毫欧，总共16节电池
    //vl_V = vl_V-((30*16)/10) *(vl_A)/100;
    vl_V = vl_V-(150/10) *(vl_A)/100;
    
    return vl_V;
}

// 10mA为单位
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
    //充放电都打开时候才算打开
    unsigned short vl_ctrl_value;
    Battery* pBat = Null;
	
	pBat = &g_Bat[bms_index];
    vl_ctrl_value = bigendian16_get((uint8*)(&pBat->bmsInfo.state));

    return vl_ctrl_value;
}

#define BATTERY_VOLTAGE_SHAKE_CNT_MAX 600
static uint32_t gl_Battery_voltage_shake_cnt;
static SwTimer gl_Battery_optime_timeout = { 0 };

// RS485 两电池状态 0:不上报,1上报
uint8_t g_Bat0_State = 0;
uint8_t g_Bat1_State = 0;
// 双电池放电算法
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


    //测试打印电压相等标志
    static unsigned char sl_both_voltage_equ_flag = 0;
    static unsigned char sl_both_voltage_diff_flag = 0;

    if(((slave_rs485_is_bat_valid(0))&&(is_battery_A_V_reg_valid(0))&&(0 == is_battery_error(0)))&&
	   ((slave_rs485_is_bat_valid(1))&&(is_battery_A_V_reg_valid(1))&&(0 == is_battery_error(1))))
	{
	    //两个电池都在线
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
            //输出电流是负值，输入时正直
            vl_current_diff = vl_bms_1_A - vl_bms_0_A;
        }
        else
        {
            vl_voltage_diff = vl_bms_1_V - vl_bms_0_V;
            //输出电流是负值，输入时正直
            vl_current_diff = vl_bms_0_A - vl_bms_1_A;
        }
		BAT_DEBUG_MSG("vol_diff:%d,cur_diff:%d\n",vl_voltage_diff,vl_current_diff);
		
		if(sl_supply_state == 1)//同时接入
        {
            if((vl_voltage_diff < 600)&&(vl_current_diff < 2500) )
            {
                gl_Battery_voltage_shake_cnt = GET_TICKS();
            }
            else if((GET_TICKS() -gl_Battery_voltage_shake_cnt) > BATTERY_VOLTAGE_SHAKE_CNT_MAX )
            {
                // 电压差大于等于6V或者电流差大于25A，变为单个接入
                sl_supply_state = 2;
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
                gl_Battery_voltage_shake_cnt = GET_TICKS();
            }
            else if((GET_TICKS() -gl_Battery_voltage_shake_cnt) > (BATTERY_VOLTAGE_SHAKE_CNT_MAX*2) )//两倍去抖时间
            {
                //两个电池都有电流，并且电流差小于20A，变为同时接入
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
		//两个电池电压接入
        if(1 == sl_supply_state)
        {
            //打开电池0的充放电开关
			if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
			{
				Bat_setDischg(&g_Bat[0], True);
				Bat_setChg(&g_Bat[0], True);
				BAT_DEBUG_MSG("Bat[0] Start DisChg&Chg\n");
			}
			//打开电池1的充放电开关
			if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 0)||
					((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 0))
			{
				Bat_setDischg(&g_Bat[1], True);
				Bat_setChg(&g_Bat[1], True);
				BAT_DEBUG_MSG("Bat[1] Start DisChg&Chg\n");
			}
            sl_bms0_vol_cmp_offset = 0;
            sl_bms1_vol_cmp_offset = 0;

			if( vl_bms_0_A > 1500 ) 
			{
				g_Bat0_State = 1 ;
				g_Bat1_State = 0 ;
			}
			else if( vl_bms_0_A > 1500 )
			{
				g_Bat0_State = 0 ;
				g_Bat1_State = 1 ;
			}
			//
			if( vl_bms_0_A > 500 && vl_bms_0_A < 12 )
			{
				g_Bat0_State = 0 ;
			}
			else if( vl_bms_1_A > 500 && vl_bms_1_A < 12 )
			{
				g_Bat1_State = 1 ;
			}
		
        }
        else if(2 == sl_supply_state)//单电池接入
        {             
            //关闭低电压的电池充放电开关
            if((vl_bms_0_V - sl_bms0_vol_cmp_offset)> (vl_bms_1_V - sl_bms1_vol_cmp_offset))
            {
                sl_bms0_vol_cmp_offset = 0;
                sl_bms1_vol_cmp_offset = 100;


                if(vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH)
                {
                    //低电压电池的充电开关是导通的，这时要求关闭
                    Bat_setChg(&g_Bat[1], False);
					BAT_DEBUG_MSG("Bat[1] Stop Chg\n");
                }
                if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //高电压电池的放电开关是关闭的，这时要求打开
                    Bat_setDischg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start DisChg\n");
                }                
                if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
                {
                    //低电压电池的放电开关是关闭的，这时要求打开
                   Bat_setDischg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start DisChg\n");
                }                
                if((vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
                {
                    //高电压电池的充电开关是关闭的，这时要求打开
                    Bat_setChg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start Chg\n");
                }
				g_Bat0_State = 1 ;
				g_Bat1_State = 0 ;
			}
			else
			{            
				sl_bms0_vol_cmp_offset = 100;
				sl_bms1_vol_cmp_offset = 0;

				if(vl_bms_0_ctrl & BMS_STATE_BIT_CHG_SWITCH)
				{
					//低电压电池的充电开关是导通的，这时要求关闭
					Bat_setChg(&g_Bat[0], False); 
					BAT_DEBUG_MSG("Bat[0] Stop Chg\n");
				}
				if((vl_bms_1_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
				{
					//高电压电池的放电开关是关闭的，这时要求打开
					Bat_setDischg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start DisChg\n");
				}
				if((vl_bms_0_ctrl & BMS_STATE_BIT_SUPPLY_SWITCH) == 0)
				{
					//低电压电池的放电开关是关闭的，这时要求打开
					Bat_setDischg(&g_Bat[0], True);
					BAT_DEBUG_MSG("Bat[0] Start DisChg\n");
				}
				if((vl_bms_1_ctrl & BMS_STATE_BIT_CHG_SWITCH) == 0)
				{
					//高电压电池的充电开关是关闭的，这时要求打开
					Bat_setChg(&g_Bat[1], True);
					BAT_DEBUG_MSG("Bat[1] Start Chg\n");
				}
				g_Bat0_State = 0 ;
				g_Bat1_State = 1 ;
			}
		}
	}
	else if((slave_rs485_is_bat_valid(0))&&(is_battery_A_V_reg_valid(0))&&
	        (0 == is_battery_error(0)))
    {
		//电池0可用
//        vl_bms_0_A = Battery_get_current(0);

        vl_bms_0_ctrl=Battery_get_switch_state(0);

        //打开电池0的充放电开关
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_0_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_0_ctrl) == 0))
		{
			Bat_setDischg(&g_Bat[0], True);
			Bat_setChg(&g_Bat[0], True);
			BAT_DEBUG_MSG("Bat[0] Start DisChg&Chg\n");
        }
		//
		g_Bat0_State = 1 ;
		g_Bat1_State = 0 ;
    }
	else if((slave_rs485_is_bat_valid(1))&&(is_battery_A_V_reg_valid(1))&&
	       (0 == is_battery_error(1)))
    {
		//电池1可用
//        vl_bms_1_A = Battery_get_current(1);

        vl_bms_1_ctrl=Battery_get_switch_state(1);

        //打开电池0的充放电开关
        if(((BMS_STATE_BIT_CHG_SWITCH&vl_bms_1_ctrl) == 0)||
                ((BMS_STATE_BIT_SUPPLY_SWITCH&vl_bms_1_ctrl) == 0))
		{
			Bat_setDischg(&g_Bat[1], True);
			Bat_setChg(&g_Bat[1], True);
			BAT_DEBUG_MSG("Bat[1] Start DisChg&Chg\n");
        }
		g_Bat0_State = 0 ;
		g_Bat1_State = 1 ;
    }
}

/*
	检测双电时，是否处于接触不良的状态
	1、当双电放电，且其中某个电池放电电流 > 15A，则认为另外一个电池接触不良
	2、当双电放电，两电流放电电流 < 15A，且其中每个电流电流 > 500mA，则认为接触良
	

	true :表示接触不良,false表示正常


Bool Bat_LooseContact( uint8_t bat )
{
	Battery* pBat = Null;
	uint16_t t_cur0 = 0 , t_cur1 = 0 ;
	// 电池接入 & 放电 
	if( g_Bat[0].presentStatus == BAT_IN && g_Bat[1].presentStatus == BAT_IN )
	{
		if( ( g_Bat[0].bmsInfo.state&0x0200 ) &&  ( g_Bat[1].bmsInfo.state&0x0200 ) )
		{
			t_cur0 = SWAP16( g_Bat[0].bmsInfo.tcurr );
			t_cur1 = SWAP16( g_Bat[1].bmsInfo.tcurr );
			// 接触不良
			// 两电池均放电，如果其中某个电流放电  > 15A，则另一个电池接触不良
			// 两电池均放电，如果其中一个电池放电电流 < 500ma，则认为其接触不良
			if( t_cur0 > 1500 && bat == 1 ) return True ;
			if( t_cur1 > 1500 && bat == 0 ) return True ;
			if( t_cur0 < 50 && bat == 0 ) return True ;
			if( t_cur1 < 50 && bat == 1 ) return True ;
		}
	}
	return False ; 
}

*/




