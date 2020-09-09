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


#include "header.h"

#if 1

#define MC3430_I2C_ADDR    0x98
#ifdef SUPPORT_DA213
#define DA213_I2C_ADDR      0x4E
#endif

#define I2C_WR  0            /* 写控制bit */
#define I2C_RD  1            /* 读控制bit */
#ifndef KM08_ST
gsensor_type g_gsensor;
#endif
gsensor_abn_t  gsensor_abn;


u16 aTriAngle[91]=
{
    10000,9998,9993,9986,9975,9961,9945,9925,9902,9876,// 0 .. 9
     9848,9816,9781,9743,9702,9659,9612,9563,9510,9455,//10 .. 19
     9396,9335,9271,9205,9135,9063,8987,8910,8829,8746,//20 .. 29
     8660,8571,8480,8386,8290,8191,8090,7986,7880,7771,//30 .. 39
     7660,7547,7431,7313,7193,7071,6946,6819,6691,6560,//40 .. 49
     6427,6293,6156,6018,5877,5735,5591,5446,5299,5150,//50 .. 59
     5000,4848,4694,4639,4383,4226,4067,3907,3746,3683,//60 .. 69
     3420,3255,3090,2923,2756,2588,2419,2249,2079,1908,//70 .. 79
     1736,1564,1391,1218,1045, 871, 697, 523, 348, 174,//80 .. 89
     0
};





#if 0


//#include "zg_all.h"

#define i2c_delay 2

U8 zg_i2c_slaveAddr_R=0x17;
U8 zg_i2c_slaveAddr_W=0x16;


#define EDGE_SENSITIVE           	KAL_TRUE
#define LEVEL_SENSITIVE          	KAL_FALSE
//#define MS_SCL	43|0x80
//#define MS_SDA	44|0x80
#define MS_SCL	43
#define MS_SDA	44

#define MS_CLK_PIN_GPIO_MODE		;
#define MS_DATA_PIN_GPIO_MODE		;
#define MS_I2C_CLK_OUTPUT			ZG_GPIO_Init(MS_SCL, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP)
#define MS_I2C_DATA_OUTPUT			ZG_GPIO_Init(MS_SDA, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP)
#define MS_I2C_DATA_INPUT		   	ZG_GPIO_Init(MS_SDA, PINDIRECTION_IN, PINLEVEL_LOW, PINPULLSEL_PULLUP)
#define MS_I2C_CLK_HIGH				ZG_GPIO_SetLevel(MS_SCL, PINLEVEL_HIGH)
#define MS_I2C_CLK_LOW				ZG_GPIO_SetLevel(MS_SCL, PINLEVEL_LOW)
#define MS_I2C_DATA_HIGH			ZG_GPIO_SetLevel(MS_SDA, PINLEVEL_HIGH)
#define MS_I2C_DATA_LOW				ZG_GPIO_SetLevel(MS_SDA, PINLEVEL_LOW)
#define MS_I2C_GET_BIT				ZG_GPIO_GetLevel(MS_SDA)

#define MS_SLAVEADDR_W  zg_i2c_slaveAddr_W
#define MS_SLAVEADDR_R  zg_i2c_slaveAddr_R
/***************************************************/

/******************************************
	us delay for software I2C
	uint:us
*******************************************/
void ZG_i2c_udelay(kal_uint32 delay)
{
	kal_uint32 count = 0;
	kal_uint32 k = 0;

	for(k = 0; k < delay; k++)
	{
        //for (count = 0; count < 30; count++)//10 OK,  30 NOT OK
        //{
           ;
        //}
    }
}

/******************************************
	software I2C start bit
*******************************************/
void ZG_i2c_start(void)
{
	MS_CLK_PIN_GPIO_MODE;
	MS_I2C_CLK_OUTPUT;
    
	MS_DATA_PIN_GPIO_MODE;
	MS_I2C_DATA_OUTPUT;
	
	MS_I2C_DATA_HIGH;
	MS_I2C_CLK_HIGH;
	ZG_i2c_udelay(20);		//20
	MS_I2C_DATA_LOW;
	ZG_i2c_udelay(10);		//10
	MS_I2C_CLK_LOW;
	ZG_i2c_udelay(10);		//10
}

/******************************************
	software I2C stop bit
*******************************************/
void ZG_i2c_stop(void)
{
	MS_I2C_CLK_OUTPUT;
	MS_I2C_DATA_OUTPUT;
	
	ZG_i2c_udelay(10);		//10
	MS_I2C_CLK_HIGH;
	ZG_i2c_udelay(10);		//10
	MS_I2C_DATA_HIGH;
}

/******************************************
	software I2C one clock
*******************************************/
void ZG_i2c_one_clk(void)
{
	ZG_i2c_udelay(5);		//5
	MS_I2C_CLK_HIGH;
	ZG_i2c_udelay(10);		//10
	MS_I2C_CLK_LOW;
	ZG_i2c_udelay(5);		//5
}

/******************************************
	software I2C read byte with ack
*******************************************/
kal_uint8 ZG_ReadByteAck(void)
{
	kal_int8 i;
	kal_uint8 data;

	MS_I2C_DATA_INPUT; 
	data = 0; 
	
	for (i=7; i>=0; i--) 
	{
		if (MS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
		}
		ZG_i2c_one_clk();
	}			                                

	MS_I2C_DATA_OUTPUT;                    
	MS_I2C_DATA_LOW;                       
	ZG_i2c_one_clk();                         

	return data;
}

/******************************************
	software I2C read byte without ack
*******************************************/
kal_uint8 ZG_ReadByteNAck(void)
{
	kal_int8 i;
	kal_uint8 data;
     //  MS_I2C_DATA_HIGH;
	MS_I2C_DATA_INPUT; 
	
	data = 0; 
	
	for (i=7; i>=0; i--) 
	{
		if (MS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
			
		}
		ZG_i2c_one_clk();
	}			                                

	MS_I2C_DATA_OUTPUT;                                           
	MS_I2C_DATA_HIGH;
	ZG_i2c_one_clk();                         
	
	return data;
}

/******************************************
	software I2C send byte
*******************************************/
void ZG_SendByte(kal_uint8 sData) 
{
	kal_int8 i;
	
	for (i=7; i>=0; i--) 
	{            
		if ((sData>>i)&0x01) 
		{               
			MS_I2C_DATA_HIGH;	              
		} 
		else 
		{ 
			MS_I2C_DATA_LOW;                  
		}
		ZG_i2c_one_clk();                        
	}		
}
/******************************************
	software I2C check ack bit
*******************************************/
kal_bool ZG_ChkAck(void)
{
	MS_I2C_DATA_INPUT;
	ZG_i2c_udelay(5);		//5
	MS_I2C_CLK_HIGH;
	ZG_i2c_udelay(5);		//5

	if(MS_I2C_GET_BIT)		//Non-ack
	{
		ZG_i2c_udelay(5);	//5
		MS_I2C_CLK_LOW;
		ZG_i2c_udelay(5);	//5
		MS_I2C_DATA_OUTPUT;
		MS_I2C_DATA_LOW;
		return KAL_FALSE;
	}
	else					//Ack
	{
	      
		ZG_i2c_udelay(5);	//5
		MS_I2C_CLK_LOW;
		ZG_i2c_udelay(5);	//5
		MS_I2C_DATA_OUTPUT;
		MS_I2C_DATA_LOW;

		return KAL_TRUE;
	}
}

/******************************************
	software I2C restart bit
*******************************************/
void ZG_Restart(void)
{
	MS_I2C_CLK_OUTPUT;
	MS_I2C_DATA_OUTPUT;
	ZG_i2c_udelay(20);
	MS_I2C_DATA_HIGH;
	ZG_i2c_udelay(10);		//10
	MS_I2C_CLK_HIGH;
	ZG_i2c_udelay(20);
	MS_I2C_DATA_LOW;
	ZG_i2c_udelay(10);		//10
	MS_I2C_CLK_LOW;
	ZG_i2c_udelay(10);		//10
}




/******************************************
	 read bytes
*******************************************/
BOOL ZG_I2C_ReadByte(kal_uint8* Data, kal_uint8 RegAddr)
{
	ZG_i2c_start();						//start bit
	ZG_SendByte(MS_SLAVEADDR_W);		//slave address|write bit
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}	
	ZG_SendByte(RegAddr);				//send RegAddr
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}

	ZG_Restart();						//restart bit

	ZG_SendByte(MS_SLAVEADDR_R);		//slave address|read bit
	if(KAL_FALSE == ZG_ChkAck())
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}

	*Data = ZG_ReadByteNAck();

	ZG_i2c_stop();						//stop bit

	//TO_DO: add debug code to display the data received

	return KAL_TRUE;
	
}

/******************************************
	BMA220 contunious read bytes
*******************************************/
BOOL ZG_I2C_ReadBytes(kal_uint8* Data, kal_uint8 RegAddr, kal_uint16 Length)
{
	kal_uint8* Data_ptr;
	kal_uint16 i;

	Data_ptr = Data;
	
	ZG_i2c_start();						//start bit
	ZG_SendByte(MS_SLAVEADDR_W);		//slave address|write bit
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}
		
	ZG_SendByte(RegAddr);				//send RegAddr
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}

	ZG_Restart();						//restart bit

	ZG_SendByte(MS_SLAVEADDR_R);		//slave address|read bit
	if(KAL_FALSE == ZG_ChkAck())
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}

	for(i=Length; i>1; i--)
	{
		*Data_ptr = ZG_ReadByteAck();	//read byte with ack
		Data_ptr++;
	}
	
	*Data_ptr = ZG_ReadByteNAck();		//read byte with non-ack to stop reading

	ZG_i2c_stop();						//stop bit

	return KAL_TRUE;
}

/******************************************
	write byte
*******************************************/
BOOL ZG_I2C_WriteByte(kal_uint8 RegAddr, kal_uint8 Data)
{
	ZG_i2c_start();						//start bit
	ZG_SendByte(MS_SLAVEADDR_W);		//slave address|write bit
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
			ZG_i2c_stop();
		return KAL_FALSE;
	}
	ZG_SendByte(RegAddr);				//send RegAddr
	if(KAL_FALSE == ZG_ChkAck())		//check Ack bit
	{
			ZG_i2c_stop();
		return KAL_FALSE;
	}
	ZG_SendByte(Data);					//send parameter
	if(KAL_FALSE == ZG_ChkAck())
	{
		ZG_i2c_stop();
		return KAL_FALSE;
	}
	ZG_i2c_stop();						//stop bit
	return KAL_TRUE;
}


s32 ZG_I2C_Init(U8 slaveAddr_R,U8 slaveAddr_W)
{
   zg_i2c_slaveAddr_R=slaveAddr_R;
   zg_i2c_slaveAddr_W=slaveAddr_W;
   return 1;
}

#endif



void gsensor_power_down(void)
{
    GSENSOR_OFF();
    ZG_GPIO_Init(ZG_GPIO43, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);  
    ZG_GPIO_Init(ZG_GPIO44, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN); 
    ZG_GPIO_SetLevel(ZG_GPIO43, PINLEVEL_LOW);
    ZG_GPIO_SetLevel(ZG_GPIO44, PINLEVEL_LOW);
    g_gsensor.power_status = POWER_OFF;
}

void gsensor_power_on(void)
{
  GSENSOR_ON();
  g_gsensor.power_status = POWER_ON;
}

#ifdef SUPPORT_DA213
u8 gsensor_getDeviceCode(void)
{
    u8 cnt;
    
    cnt = 4;
    while(--cnt)
    {
        if(cnt%2)
        {
            ZG_I2C_Init( DA213_I2C_ADDR|I2C_RD,  DA213_I2C_ADDR|I2C_WR);
            g_gsensor.PCode = 0x00;
            ZG_I2C_ReadByte(&g_gsensor.PCode, NSA_REG_WHO_AM_I);
            if(g_gsensor.PCode == 0x13)
            {
                return 0;
            }
        }
        else
        {
            ZG_I2C_Init( MC3430_I2C_ADDR|I2C_RD , MC3430_I2C_ADDR|I2C_WR);  
            g_gsensor.PCode = 0x00;
            ZG_I2C_ReadByte(&g_gsensor.PCode, MMA_RO_WHOAMI);
            if(((g_gsensor.PCode&0xF1) == 0x10) || (g_gsensor.PCode==0x39))
            {
                return 0;
            };
        }              
    }

    return 1;
}
#endif

u8 gsensor_readId(void)
{
    u8 id_buff[2];

    #ifdef SUPPORT_DA213
    if(g_gsensor.PCode==0x13)
    {
        ZG_I2C_ReadByte(&id_buff[0], NSA_REG_WHO_AM_I);
    }    
    else
    #endif
    {    
        ZG_I2C_ReadByte(&id_buff[0], MMA_RO_WHOAMI);
        
    }
    return (id_buff[0]);
}

/**
 * 初始化参数
 */
void gsensor_init_par(void)
{
    g_gsensor.sta = 0;         // 工作状态,正常，异常
    g_gsensor.shake_evt = 0;
    g_gsensor.max_dif = 0;     // 最大XYZ矢量和差值
    g_gsensor.minor_max_dif = 0;
    #ifndef GSENSOR_BUG_20170509
    g_gsensor.vec_len = 0;     // 矢量和缓冲区计数    
    #endif
    // 清除XYZ轴值
    gsensor_abn.abn_flg = 0;
    gsensor_abn.xyz_buff[0] = 0;
    gsensor_abn.xyz_buff[1] = 0;
    gsensor_abn.xyz_buff[2] = 0;
    // MC3430读取异常复位不能清除
    if (GSENSOR_SAVE_FLG != gsensor_abn.flg)
    {
        gsensor_abn.id_err_cnt = 0;
        gsensor_abn.xyz_err_cnt = 0;
        gsensor_abn.tx_id_err_cnt = 0;
        gsensor_abn.tx_xyz_err_cnt = 0;
    }
}

/**
 * MC3430初始化
 * @param type
 */
void gsensor_Init(void)
{
    //---
    gsensor_power_on();
    
    gsensor_init_par();
    #ifdef GPS_FIXED_PRECISION
    GPS.fixed_ignore = ACTIVE_NONE; // GPS更新位置
    #endif
    //--读取设备ID
    #ifdef SUPPORT_DA213
    if(gsensor_getDeviceCode())
    {
        g_gsensor.sta = 1;     // 异常
        gsensor_abn.tx_id_err_cnt += 1;
        return;
    }
    #else
    g_gsensor.PCode = gsensor_readId();
    #endif

    #ifdef SUPPORT_DA213
    if (g_gsensor.PCode == 0x13)
    {
        ZG_I2C_WriteByte(NSA_REG_POWERMODE_BW,  0x90);        //suspend mode
        ZG_I2C_WriteByte(NSA_REG_G_RANGE, 0x0C);                // 8bit、2Gmode
        ZG_I2C_WriteByte(NSA_REG_ODR_AXIS_DISABLE, 0x07);   //125Hz
        ZG_I2C_WriteByte(NSA_REG_INT_PIN_CONFIG, 0x05);      //set int_pin level
        ZG_I2C_WriteByte(NSA_REG_INT_LATCH, 0x66);            //clear latch and set latch mode
        ZG_I2C_WriteByte(NSA_REG_POWERMODE_BW, 0x10);        //normalmode,125HZ
    }
    else
    #endif
    #ifdef SUPPORT_3413       
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        ZG_I2C_WriteByte(0x07, 0x43);
        ZG_I2C_WriteByte(0x08, 0x09); //128sample/sec //0x00);//32sample/sec   
        ZG_I2C_WriteByte(0x06, 0x00); //无中断使能
        ZG_I2C_WriteByte(0x20, 0x02); //配置成8位分辨率
        ZG_I2C_WriteByte(0x07, 0x41);
    }
    else
    #endif
    {
        ZG_I2C_WriteByte(0x07, 0x43);
        ZG_I2C_WriteByte(0x05, 0x00); //休眠计数器为0        
        ZG_I2C_WriteByte(0x08, 0x00); //128sample/sec
        ZG_I2C_WriteByte(0x06, 0x00); //无中断使能
        ZG_I2C_WriteByte(0x07, 0x41);
    }
}

/**
 * 开根号
 * @param x
 */
unsigned long int_sqrt(unsigned long x)
{
    unsigned long op, res, one;

    op = x;
    res = 0;

    one = 1UL << 30;
    while (one > op)
        one >>= 2;

    while (one != 0) {
        if (op >= res + one) {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res /= 2;
        one /= 4;
    }
    return res;
}

/**
 * 计算XYZ单轴平方值
 * @param xyz
 * @return
 */
static u32 mc3430_calclulate_xyz_mg(u8 xyz)
{
    u32 xyz_mg = 0;

    if (xyz & 0x80)
    {
        xyz = ~xyz;
        xyz += 1;
    }

    xyz_mg = xyz;
    #ifdef SUPPORT_DA213
    if (g_gsensor.PCode == 0x13)
    {
        xyz_mg = (xyz_mg * 156)/10;    // 15.6*128 = 2000mg 
    }
    else
    #endif
    #ifdef SUPPORT_3413
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        xyz_mg = (xyz_mg * 156)/10;    // 15.6*128 = 2000mg 
    }
    else
    #endif
    {
        xyz_mg = (xyz_mg * 117)/10;    // 11.7*128 = 1500mg 
    }
    xyz_mg = xyz_mg * xyz_mg;

    return xyz_mg;
}

/**
 * 异常处理
 */
void mc3430_checkErr(void)
{
    gsensor_abn.id_err_cnt++;
    gsensor_abn.flg = GSENSOR_SAVE_FLG;
    /*if((gsensor_abn.id_err_cnt == 100) && (g_gsensor.PCode == 0x13))
    {
        ptmsg("###:初始化I2C DA213地址......\r\n");
        ZG_I2C_Init( DA213_I2C_ADDR|I2C_RD,  DA213_I2C_ADDR|I2C_WR);
    }
    if((gsensor_abn.id_err_cnt == 101) && ((g_gsensor.PCode == 0x39) || ((g_gsensor.PCode&0xF1) == 0x10)))
    {
        ptmsg("###:初始化I2C MC3430/3413地址......\r\n");
        ZG_I2C_Init( MC3430_I2C_ADDR|I2C_RD , MC3430_I2C_ADDR|I2C_WR);
    }*/
     
    if (gsensor_abn.id_err_cnt >= GSENSOR_ID_ERROR_CNT)
    {
    	if (g_gsensor.PCode == 0x13)
    	{
    		ZG_I2C_WriteByte(NSA_REG_SPI_I2C, 0x24);   //DA213 Gsensor软件复位
    	}
        ptmsg("###:Gsensor读取异常: %d\r\n", gsensor_abn.id_err_cnt);
        gsensor_power_down();
        ZG_StartTimer(TIMER_ID_GSENSOR_INIT, TIM_GEN_1SECOND*2, gsensor_Init);
        g_gsensor.sta = 1;     // 异常
        g_AppInfo.gsensor.workState = 1;
        gsensor_abn.id_err_cnt = 0;
        gsensor_abn.tx_id_err_cnt += 1;
    }
}

/**
 * XYZ轴值异常处理
 * @param xyz
 * @return
 */
static u8 mc3430_xyz_abnormal(u8 *xyz)
{
    u8 x=0,y=0,z=0;

    #ifdef SUPPORT_DA213
    if (g_gsensor.PCode == 0x13)
    {
        x = *(xyz + 1);
        y = *(xyz + 3);
        z = *(xyz + 5);
    }
    else
    #endif
    #ifdef SUPPORT_3413
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        x = *xyz ;
        y = *(xyz + 2);
        z = *(xyz + 4);
    }
    else
    #endif
    {
        x = *xyz ;
        y = *(xyz + 1);
        z = *(xyz + 2);
    }
    //-----

    if((x == 0xFF) && (y == 0xFF) && (z == 0xFF))
    {
        gsensor_abn.xyz_err_cnt_ff++;
        if(gsensor_abn.xyz_err_cnt_ff > 499) //20ms * 500
        {
            ptmsg("###:Gsensor三轴值异常: %d\r\n  ", gsensor_abn.xyz_err_cnt);
            gsensor_power_down();
            ZG_StartTimer(TIMER_ID_GSENSOR_INIT, TIM_GEN_1SECOND*2, gsensor_Init);
            g_gsensor.sta = 0x01;  // 异常
            gsensor_abn.abn_flg = 0x01;
            gsensor_abn.xyz_err_cnt = 0;
            gsensor_abn.tx_xyz_err_cnt += 1;
            gsensor_abn.xyz_err_cnt_ff = 0;
            return 1;
        }
    }
    else
    {
        gsensor_abn.xyz_err_cnt_ff = 0;
    }
    
    if ((x != gsensor_abn.xyz_buff[0]) || (y != gsensor_abn.xyz_buff[1]) || (z  != gsensor_abn.xyz_buff[2]))
    {
        gsensor_abn.xyz_buff[0] = x;
        gsensor_abn.xyz_buff[1] = y;
        gsensor_abn.xyz_buff[2] = z;
        gsensor_abn.xyz_err_cnt = 0;
        gsensor_abn.abn_flg = 0;

        #if 0//def debug_gsensor
        ptmsg("xyz[%d,%d,%d],", gsensor_abn.xyz_buff[0],gsensor_abn.xyz_buff[1],gsensor_abn.xyz_buff[2]);
        #endif
    }
    else
    {
        gsensor_abn.xyz_err_cnt++;
        gsensor_abn.flg = GSENSOR_SAVE_FLG;
        if (gsensor_abn.xyz_err_cnt >= GSENSOR_ABNORMAL_TIM)
        {
            ptmsg("###:Gsensor三轴值异常: %d\r\n  ", gsensor_abn.xyz_err_cnt);
            gsensor_power_down();
            ZG_StartTimer(TIMER_ID_GSENSOR_INIT, TIM_GEN_1SECOND*2, gsensor_Init);
            g_gsensor.sta = 0X01;  // 异常
            gsensor_abn.abn_flg = 0x01;
            gsensor_abn.xyz_err_cnt = 0;
            gsensor_abn.tx_xyz_err_cnt += 1;
            return 1;
        }

        if (0x01 == gsensor_abn.abn_flg)
            return 1;
    }

    return 0;
}


#ifdef GSENSOR_BUG_20170509
void getShakeValue(u16 xyzVacData)
{
    u16 _shakeValue;
    static u16 s_shakeTamp=0;
    static u8  s_shakeCnt=0;
    
    static u16 s_studyTamp=0;
    static u8  s_studyCnt=0;

    //----学习静态的加速度值
    if((xyzVacData>900) &&(xyzVacData<1100) )
    {
        s_studyTamp+=xyzVacData;
        s_studyCnt++;

        if(s_studyCnt>=40)
        {
            g_gsensor.staticValue = s_studyTamp/s_studyCnt;
            s_studyTamp=0;
            s_studyCnt=0;
        }
    }
    else
    {
        s_studyTamp=0;
        s_studyCnt=0;
    }
    //判断异常静态值
    if((g_gsensor.staticValue<900) ||(g_gsensor.staticValue>1100) ) {g_gsensor.staticValue=1000;}
    //-----计数震动值
    _shakeValue = (xyzVacData>g_gsensor.staticValue)?(xyzVacData-g_gsensor.staticValue):(g_gsensor.staticValue-xyzVacData);
    //----判断震动
    if((g_gsensor.sonser_zdvlaue>0) && (_shakeValue > g_gsensor.sonser_zdvlaue))
    {
        //获取最大值
        if(s_shakeTamp<_shakeValue)
        {
            s_shakeTamp = _shakeValue;
        }
        //过滤突变点
        if(s_shakeCnt >= 1)
        {
            if(g_gsensor.max_dif<s_shakeTamp)
            {
                g_gsensor.max_dif = s_shakeTamp;   
                g_gsensor.minor_max_dif = g_gsensor.max_dif;
            }
            s_shakeTamp=0;
        }
        else
        {
            s_shakeCnt++;
        }
    }
    else
    {
        if(g_gsensor.max_dif<_shakeValue)
        {
            g_gsensor.max_dif = _shakeValue;
            g_gsensor.minor_max_dif = g_gsensor.max_dif;
        }
        
        s_shakeTamp=0;
        s_shakeCnt = 0;
    }
}
#else
/**
 * 获取XYZ矢量和最大值、最小值、最大值前一个值
 * @param mc3430
 * @param p_out
 * @return
 */
u16 GetMaxV(t_gsensor_type *gsensor, u16 *p_out)
{
    u8 i = 0;
    u16 iMax = 0;
    u16 iMin = 0;
    u16 minor_max = 0;

    iMax = gsensor->vec_buff[0];
    iMin = gsensor->vec_buff[0];
    minor_max = 0;
    for (i = 1; i < GSENSOR_BUFF_LEN; i++)
    {
        if (gsensor->vec_buff[i] > iMax)
        {
            minor_max = iMax;
            iMax = gsensor->vec_buff[i];
        }
        else if (gsensor->vec_buff[i] < iMin)
        {
            iMin = gsensor->vec_buff[i];
        }
        else if (gsensor->vec_buff[i] > minor_max)
        {
        	minor_max = gsensor->vec_buff[i] ;
        }
    }

    *p_out = (minor_max - iMin);

    #ifdef debug_gsensor
    g_max = iMax;
    g_min = iMin;
    #endif
    return (iMax - iMin);
}

#endif


#if 0//def SUPPORT_3413
#define gsensor_filter(last_data, data)		((( 60* (last_data)) + (40 * (data))) / 100)

u32 MC3XXX_HPF(u32 filter_data)
{
    static u32 pre_data=0;

    if(pre_data>0)
    {
        pre_data = gsensor_filter(filter_data, pre_data);
    }
    else
    {
        pre_data = filter_data;
    }
    
    return pre_data;
}
#endif


/************************************************************************
    Function :
    Description : 静止状态上发一次0速度点
        
    Parameter : 
    Return : 
    Author: 
    Date:  Aug-11-2016
************************************************************************/
void standby_event_proc(void)
{
  #ifdef SEG_PROTOCOL
  u8 CellFlag=0;
  #endif
  log_msg();

  if(check_gps_buff() == UOP_FAIL)
  {
    log_msg();
    return;
  }
  
  if(GPS.has_location != 0xAA)
  {
    log_msg();
    return;
  }
  
  GPS.locat_flag = 0xCC;
  ptmsg("\r\n 静止状态上发一次0速度点 standby_eventB:work:%d flg:%x ", StatInfo.work_status,GPS.locat_flag);
  #ifndef SEG_PROTOCOL
  JT_send_oneNow(0,UPLD_LAST_LOCAT);
  #else
  CellFlag=(g_Para.cell_info_sw==CELL_INFO_ON)?(BasicIs):(BasicNo);
  FindCarFunction(GprsType,Platf_93,CellFlag,&ProtParam,"ITV");
  #endif
  GPS.locat_flag = 0x00;
}


/**
 * 读取XYZ矢量和、最大差值、最大值前一个点与最小值差值
 */
void gsensor_getAccelerate(void)
{
    static u8 time=0;
    static u8 buff[6];
    u8 i = 0;
    u8 cMMA_ID = 0;
    u16 limit_v = 0;
    u32 xyz_vec_data = 0;
    //static u8 idx = 0;
    //static u8 cnt = 4;


    /*** 20ms  *****/
    if (++time < 2)
    //if (++time < 4)
        return;
    time = 0;

    #ifdef SUPPORT_FACTORY_TEST
    if(fac_test.mode == ENABLE_TEST_MODE)
    {
      return;
    }
    #endif

    /****  read id state  ****/
    if(g_gsensor.sta == 1)
    {

        #if 0

            if(cnt%2)
            {
                ZG_GPIO_SetLevel(ZG_GPIO43, PINLEVEL_LOW);
                ZG_GPIO_SetLevel(ZG_GPIO44, PINLEVEL_LOW);
            }
            else
            {
                ZG_GPIO_SetLevel(ZG_GPIO43, PINLEVEL_HIGH);
                ZG_GPIO_SetLevel(ZG_GPIO44, PINLEVEL_HIGH);
            }   
        if(cnt > 0)
        {
            cnt --;
        }
        else
        {
            cnt = 4;
        }
        return;
        #else
        cMMA_ID = gsensor_readId();
        if ((cMMA_ID != 0x39) 
            #ifdef SUPPORT_3413
            && ((cMMA_ID&0xF1) != 0x10)
            #endif
            #ifdef SUPPORT_DA213
            &&(cMMA_ID != 0x13)
            #endif
            )
        {
            mc3430_checkErr();
            return;
        }
        #endif
    }

    #ifdef SUPPORT_DA213
    if (g_gsensor.PCode == 0x13)
    {
        i = ZG_I2C_ReadBytes(&buff[0], 0x02, 6);
    }
    else
    #endif
    #ifdef SUPPORT_3413
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        i = ZG_I2C_ReadBytes(&buff[0], 0x0d, 6);
    }
    else
    #endif
    {
        i = ZG_I2C_ReadBytes(&buff[0], 0x00, 3);
    }    
    if (i == KAL_FALSE)
    {
        mc3430_checkErr();
        return;
    }
	
    // XYZ轴异常处理
    i = mc3430_xyz_abnormal(&buff[0]);
    if (0 != i)
    {
        g_AppInfo.gsensor.workState = 1;
        return;
    }
    g_AppInfo.gsensor.workState = 0;  
    JudgeTermiWorkSta(buff[1],buff[3],buff[5]);

    // 正常
    g_gsensor.sta = 0;
    gsensor_abn.id_err_cnt = 0;
    
    #ifdef SUPPORT_DA213
    if (g_gsensor.PCode == 0x13)
    {
        xyz_vec_data = mc3430_calclulate_xyz_mg(buff[1]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[3]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[5]);
    }
    else
    #endif
    #ifdef SUPPORT_3413
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        xyz_vec_data = mc3430_calclulate_xyz_mg(buff[0]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[2]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[4]);
    }
    else
    #endif
    {
        xyz_vec_data = mc3430_calclulate_xyz_mg(buff[0]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[1]);
        xyz_vec_data += mc3430_calclulate_xyz_mg(buff[2]);
    }
    xyz_vec_data = int_sqrt(xyz_vec_data);
    
    #if 0//def SUPPORT_3413
    if ((mc3430.PCode&0xF1) == 0x10)
    {
        xyz_vec_data = MC3XXX_HPF(xyz_vec_data);
    }
    #endif
    
    #ifdef GSENSOR_BUG_20170509
    getShakeValue((u16)xyz_vec_data);
    #else
    if (g_gsensor.vec_len < GSENSOR_BUFF_LEN)
    {
        g_gsensor.vec_buff[g_gsensor.vec_len++] = (u16) (xyz_vec_data);
    }

    
    /*idx++;
    if(idx > 24)
    {
        ptmsg("xyz[%d,%d,%d],", gsensor_abn.xyz_buff[0],gsensor_abn.xyz_buff[1],gsensor_abn.xyz_buff[2]);
        idx = 0;
    }*/

    if (g_gsensor.vec_len < GSENSOR_BUFF_LEN)
        return;

    
    
    /*---------------------以下是1秒-------------------------------*/
    xyz_vec_data = GetMaxV(&g_gsensor, &limit_v);
    #ifdef SUPPORT_3413
    if ((g_gsensor.PCode&0xF1) == 0x10)
    {
        xyz_vec_data = (xyz_vec_data*70)/100;
    }
    #endif

    
    //---
    
    g_gsensor.vec_len = 1;

    #ifdef debug_gsensor
    if((G_sensor_xyz(&g_gsensor) > 0) && (xyz_vec_data>g_Para.shake_threshold) && (limit_v>g_Para.shake_threshold))
    {
        g_gsensor.vec_buff[0] = g_gsensor.vec_buff[GSENSOR_BUFF_LEN-1];
        ptmsg("--->G-Sensor差值_异常过滤:%d/%dmg,", xyz_vec_data, limit_v);
        //ptmsg("max:%d,min:%d,", g_max, g_min);
        return;
    }
    #endif

    g_gsensor.vec_buff[0] = g_gsensor.vec_buff[GSENSOR_BUFF_LEN-1];
    
    //ptmsg("333333333333333333333333333333333\r\n  ");
    // 最大矢量和差值
    g_gsensor.max_dif = xyz_vec_data;
    g_gsensor.minor_max_dif = limit_v;
    #endif  

    if ((g_gsensor.max_dif>g_Para.shake_threshold) && (g_gsensor.minor_max_dif>g_Para.shake_threshold))
    {
        g_gsensor.iot_shake_continue++;   //持续10次震动才报急加速急减速
        
        g_gsensor.shake_evt = 1;        
        ptmsg("产生振动max_dif %d, shake_threshold %d, minor_max_dif %d, iot_shake_continue %d\r\n", 
                    g_gsensor.max_dif, g_Para.shake_threshold, g_gsensor.minor_max_dif, g_gsensor.iot_shake_continue);
        #ifdef GPS_FIXED_PRECISION
        // GSensor大于60mg实时更新位置
        GPS.fixed_ignore = ACTIVE_NONE; // GPS更新位置
        #endif
        #ifdef debug_gsensor
        ptmsg("--->G-Sensor差值:%d/%dmg,", g_gsensor.max_dif, g_gsensor.minor_max_dif);
        ptmsg("震动阈值:%dmg,", g_Para.shake_threshold);
        ptmsg("xyz[%d,%d,%d],", gsensor_abn.xyz_buff[0],gsensor_abn.xyz_buff[1],gsensor_abn.xyz_buff[2]);
        ptmsg("max:%d,min:%d,", g_max, g_min);
        #endif
    }
    else
    {       
        g_gsensor.iot_shake_continue = 0;
        #ifdef GPS_FIXED_PRECISION
        // GSensor小于60mg并且定位速度小于5公里，GPS不更新。下次更新GSensor必须大于60mg
        if (g_GpsSpeed < 5)
        {
            GPS.fixed_ignore = ACTIVE_OK; // 不更新GPS位置
        }
        #endif
    }
    #ifdef IMPACT_ALARM_ENABLE
    CheckImpack();
    #endif
    i=0;

    #ifdef GSENSOR_SHARK_ALARM
    gsensor_shaking_alarm();
    #endif
    //----
    if(i==1)
    {
        ZG_StartTimer(TIMER_ID_ALARM_UPLD, TIM_GEN_1MS, alarm_event_proc);
    }
}
u8 gsensor_shaking_alarm(void)
{
    #ifdef RUN_CAR_TEST_LOG
    u8 buffer[1000] = {0};
    u32 len = 0;
    s32 ret = 0;
    #endif
    
    //if((g_gsensor.max_dif>1500) && (g_gsensor.minor_max_dif>1200))
    if ((g_gsensor.max_dif > (g_Para.shakeAlarmThresh + 300)) 
        && (g_gsensor.minor_max_dif > g_Para.shakeAlarmThresh))
    {
        if(g_Para.sharkDetectSW == 1 && app.shake_alarm_timeout == 0)
        {
            if(((g_GpsSpeed<20) && (GPS.Buf[6]=='A')) || (GPS.Buf[6]!='A'))
            {
                app.shake_alarm = 1;
                ptmsg("振动报警上报g_GpsSpeed %d\r\n", g_GpsSpeed);
                g_AppInfo.gsensor.shakeState = 1;  
                ZG_StartTimer(TIMER_ID_ALARM_UPLD, TIM_GEN_1MS, alarm_event_proc);
                app.shake_alarm_timeout = 10;       //间隔10s还有振动则再次上报

                #ifdef RUN_CAR_TEST_LOG
                len = log_data_pkt(buffer, GSENSOR_SHARK_REPORT);

                if(m_socket.update > 0)
                {
                    ret = packet_data_send(m_socket.update, buffer, len);
                    if (ret == len)
                    {
                        StatInfo.log_upload++;
                    }
                }
                #endif
            }
        }
    }
    return UOP_OK;
}
/**
 * 
 */
void gsensor_prj(void)
{    
    static u8 time_1S=0;
    u8 status=0;
    u16 motionless_time;
    u32 idx;

    #ifdef SUPPORT_FACTORY_TEST
    if(fac_test.mode == ENABLE_TEST_MODE)
    {
      return;
    }
    #endif

    /*** 1S  *****/
    if (++time_1S < 100)
        return;
    time_1S = 0;
    
    if ((g_gsensor.shake_evt) || ((g_gsensor.sta != 0)&&(g_accCkMod == STA_ACC_ON)))
    {
        g_gsensor.shake_evt = 0;
        // 振动处理
        g_gsensor.shake_det_tim++;
        g_gsensor.motionless_tim = 0;
        
        if (g_gsensor.shake_det_tim >= 1) // 5
        {
            if (StatInfo.work_status != STEP_WORK)
            {
                StatInfo.work_status = STEP_WORK;

                if(g_gsensor.sta != 0)
                {
                    ptmsg("\r\nGsensor异常强制进入工作状态\r\n");
                }
                else
                {
                    ptmsg("\r\n进入正常工作状态:eco_tim:%d soc:%d,%d.%d.%d.%d\r\n",
                        StatInfo.eco_time,
                        m_socket.tcp, m_socket.udp,
                        m_socket.update,m_socket.upd_admin,
                        m_socket.upd_server);
                }

                #ifdef ACC_CHECK_BY_GSENSOR
                if(g_accCkMod != STA_ACC_ON)
                {
                    JT.AccOnCnt=0;
                    JT.AccOffCnt=0;
                }
                #endif
                GPS.flag_loc_grade = 0;
                GPS.location_tim = 0;
                GPS.locat_flag = 0x00; // Sep-01-2016 解决休眠起来后上传位置数据，定位到不定位
                GPS.has_location = 0;

                #if 0
                ZG_SleepDisable();
                ZG_StartTimer(TIMER_ID_GPS_ON, TIM_GEN_10MS, gps_power_on);
                #else
                tbox_stop_sleep();
                ZG_StartTimer(TIMER_ID_GPS_ON, TIM_GEN_10MS, gps_power_on);
                #endif
                
              #ifdef SUPPORT_BSJ_EPO
                if (StatInfo.eco_time >= 60*10)
                {
                    ptmsg("\r\n静止%d分钟，重发EPO\r\n", StatInfo.eco_time/60);
                    
                    status = check_usr_epo_valid();
                    if ((status==UOP_OK) || (status==UOP_EXIST))
                    {
                        log_msg();
                        ZG_StartTimer(TIMER_ID_EPO_CMD, TIM_GEN_100MS*40, epo_send_init);
                    }
                }
              #endif
                mtk_info.req_charge_bat = 0x01;
                gprs.LinkCount = 0; // 静止到运动，加快拨号速度
            }
            
            StatInfo.eco_time = 0;
            g_gsensor.sleep_time = 0;
            
        }
        
        StatInfo.eco_time = 0;
        g_gsensor.sleep_time = 0;
    }
    else
    {
        #if 1 // ID为0一直为运动状态 Aug-24-2016
        
        if (((is_valid_sdata(&g_Para.strSim[1],11,'0',&idx)) == 0)
          || ((is_valid_sdata(&g_Para.strSim[0],12,0x00,&idx)) == 0))
        {
            if (StatInfo.eco_time > 7)
            {
                ptmsg("--->strSim %s \r\n", &g_Para.strSim[1]);
                StatInfo.eco_time = 0;
                g_gsensor.sleep_time = 0;
                StatInfo.work_status = STEP_WORK;  // May-11-2017
            }
        }
        #endif
        
        #if 1 //  Sep-05-2016
        if (g_Para.AOffUpTim == 209)
        {
            if (g_Para.HeartTim == 176)
            {
                StatInfo.eco_time = 0;
                g_gsensor.sleep_time = 0;
            }
        }
        #endif
        
        g_gsensor.shake_det_tim = 0;
        g_gsensor.motionless_tim++;
        if (g_gsensor.motionless_tim >= 100) // 静止100秒，清除里程计数
        {
            g_gsensor.distance = 0;
        }
        
        StatInfo.eco_time++;
        
      #if 1
        if ((g_gsensor.speed_shake_tim>=5) && (g_GpsSpeed>=10))
        {
            if (StatInfo.work_status != STEP_WORK)
            {
                log_msg();
                StatInfo.work_status = STEP_WORK;
                ptmsg("\r\n有速度进入正常工作状态 速度%d 检测时间:%d, eco_tim:%d \r\n",
                    g_GpsSpeed, g_gsensor.speed_shake_tim,StatInfo.eco_time);

                tbox_stop_sleep();
                ZG_StartTimer(TIMER_ID_GPS_ON, TIM_GEN_10MS, gps_power_on);

                #ifdef ACC_CHECK_BY_GSENSOR
                if(g_accCkMod != STA_ACC_ON)                
                {
                    JT.AccOnCnt=0;
                    JT.AccOffCnt=0;
                }
                #endif
              #ifdef SUPPORT_BSJ_EPO
                if (StatInfo.eco_time >= 60*10)
                {
                    ptmsg("\r\n静止%d分钟-重发EPO\r\n", StatInfo.eco_time/60);
                    
                    status = check_usr_epo_valid();
                    if ((status==UOP_OK) || (status==UOP_EXIST))
                    {
                        log_msg();
                        ZG_StartTimer(TIMER_ID_EPO_CMD, TIM_GEN_100MS*40, epo_send_init);
                    }
                }
              #endif
                mtk_info.req_charge_bat = 0x01;
            }
            
            //if (StatInfo.eco_time > 10)
            {
                StatInfo.eco_time = 0;
            }
            //if (kks.motionless_tim > 4)
            {
                g_gsensor.motionless_tim = 0;
            }
            
        }
        
        if (g_Para.UploadTim == 133)
        {
            motionless_time = 1*60;
        }
        else
        {
			#ifdef SEG_PROTOCOL
			if(bACC==ACC_OFF)
			{
				motionless_time = 1*60;
			}
			else
			{
				motionless_time = 5*60;
			}
			#else
            motionless_time = 5*60;
			#endif
        }
        
        if (StatInfo.eco_time >= motionless_time)  // (5*60)  for test (5*60*60)
        {
            if ((g_accCkMod != STA_ACC_ON)&&(StatInfo.work_status != STEP_SLEEP))
            {
 //               StatInfo.work_status = STEP_ECO_STANDBY;
                
                ptmsg("\r\n进入静止状态 soc:%d,%d.%d.%d.%d  work_status %d accstatus %d\r\n",
                    m_socket.tcp, m_socket.udp,
                    m_socket.update,m_socket.upd_admin,
                    m_socket.upd_server, StatInfo.work_status, g_accCkMod);
                    
               if(g_Para.powerMod== 1)    
               { 
                    ck_bsj_gps_av = 0;
                    mtk_info.req_charge_bat = 0x00;
                    //----
                    if((GPS.locat_flag == 0xAA)||(GPS.locat_flag == 0xBB))
                    { // 有效定位
                      GPS.has_location = 0xAA;
                    }      
                    
                    #ifdef GPS_FIXED_PRECISION
                    GPS.power_on_flag = ACTIVE_OK;
                    GPS.fixed_timer = 0;
                    #endif 
                    #if 0 //def ACC_CHECK_BY_GSENSOR
                    if((g_accCkMod != STA_ACC_ON)&&(StatInfo.work_status != STEP_SLEEP))
                    #endif
                    {
                        #if 0
                        ZG_StartTimer(TIMER_ID_GPS_OFF, TIM_GEN_10MS*3, gps_power_off);
                        StatInfo.work_status = STEP_SLEEP;
                        ZG_SleepEnable();
                        ZG_StartTimer(TIMER_ID_LAST_LOCAT_EVENT, TIM_GEN_1SECOND*2, standby_event_proc); // 15
                        #else
                        ZG_StartTimer(TIMER_ID_GPS_OFF, TIM_GEN_10MS*3, gps_power_off);
                        StatInfo.work_status  = STEP_SLEEP;
                        ZG_StartTimer(TIMER_ID_LAST_LOCAT_EVENT, TIM_GEN_1SECOND*2, standby_event_proc); // 15
                        #endif
                    }
                                    
                    GPS.locat_flag = 0x00; // Sep-01-2016 解决休眠起来后上传位置数据，定位到不定位
                }
            }
        }
      #endif
        
    }
    
#if 0    
    if (StatInfo.work_status == STEP_WORK)
    {
        mtk_info.req_charge_bat = 0x01;
        mtk_info.running_status = 0x01;        
    }
    else
    {
        #ifdef CFG_CHARGEMOD
        mtk_info.req_charge_bat = (g_Para.chargeMod==0)?0x00:0x01;
        #else
        mtk_info.req_charge_bat = 0;
        #endif

        mtk_info.running_status = 0x00;

        if(g_Para.powerMod== 1)    
        {
            gps_power_off();
            GPS.LongTimeNoA = 0; // Sep-13-2016 解决进入运动状态后产生一次GPS复位重启               
            #if 1 // Aug-15-2016 Zhou require  speed reset only in eco standby mode
            GPS.Buf[6] = 'V';
            GPS.Buf[26] = '0';
            GPS.Buf[27] = '0';
            GPS.Buf[28] = '0';
            GPS.Buf[43] = '0';
            g_GpsSpeed = 0;
            gpsDirection = 0; // Dec-07-2016
            #endif
        }
    }
#endif    
}
#define OK                    1
#define ERR                   2   
void GetGsensorVal(u8 xyz,u32 *pAcceVlu,u16 *signVal)
{
    u32 xyz_mg = 0;
    if (xyz & 0x80)
    {
        xyz = ~xyz;
        xyz += 1;
		*signVal|=0x8000;
    }
	else
	{
		*signVal=0x0000;
	}
    xyz_mg = xyz;

    *pAcceVlu = (xyz_mg * 156)/10;    // 15.6*128 = 2000mg 
}
s16 GetAxisVector(u16 mVal,u16 SignFlag)
{
    s16 mRet = 0;

    mRet =  mVal & 0x7fff;
    if(SignFlag & 0x8000)
        mRet = - mRet;
    return(mRet);
}

u16 SetAppInfoXYZValue(u16 mVal,u16 SignFlag)
{
    u16 retVal = 0;

    retVal = mVal & 0x7fff;
    if(SignFlag & 0x8000)
    {
        retVal |= 0x8000;
    }
    
    return retVal;
}

u8 JudgeTermiWorkSta(u8 x,u8 y,u8 z)
{
	u8 cSleepAction = 0;
	static u32 s_dwPreXdir = 0;
	static u32 s_dwPreYdir = 0;
	static u32 s_dwPreZdir = 0;
	u16 SignX_Dir;
	u16 SignY_Dir;
	u16 SignZ_Dir;
	static u8 s_Timer = 0,P_OStateG3_Cnt=0;
	if (++s_Timer < 4) 
	{
		return (0);
	}
	s_Timer=0;

	//读X方向加速度
	GetGsensorVal(x,&StuMMA.Xdir,&SignX_Dir);

	//读Y方向加速度

	GetGsensorVal(y,&StuMMA.Ydir,&SignY_Dir);

	//读Z方向加速度
	GetGsensorVal(z,&StuMMA.Zdir,&SignZ_Dir);

	g_AppInfo.gsensor.xAxisValue = SetAppInfoXYZValue(StuMMA.Xdir, SignX_Dir);
      g_AppInfo.gsensor.yAxisValue = SetAppInfoXYZValue(StuMMA.Ydir, SignY_Dir);
      g_AppInfo.gsensor.zAxisValue = SetAppInfoXYZValue(StuMMA.Zdir, SignZ_Dir);
	
	if ((s_dwPreXdir != StuMMA.Xdir)||(s_dwPreYdir != StuMMA.Ydir)||(s_dwPreZdir != StuMMA.Zdir))
	{
	    // 400ms
	    if(++P_OStateG3_Cnt >= 2)// 5*80=400MS
	    {
	        P_OStateG3_Cnt = 0;
	        str_mStduy.mCount++;
	        str_mStduy.mxVal = GetAxisVector(StuMMA.Xdir,SignX_Dir);
	        str_mStduy.myVal = GetAxisVector(StuMMA.Ydir,SignY_Dir);
	        str_mStduy.mzVal = GetAxisVector(StuMMA.Zdir,SignZ_Dir); 
			if(StuMMA.MMAAdjust==0xAA)
			{
				StuMMA.ReadMMAVal=0xAA;
				ptmsg("JudgeTermiWorkSta adjust OK\r\n");
			}
	    }
	}
	s_dwPreXdir = StuMMA.Xdir;
	s_dwPreYdir = StuMMA.Ydir;
	s_dwPreZdir = StuMMA.Zdir;
	return cSleepAction;   
}
u8 LookUpTable(u16 * pData,u8 Lenth,u16 mVal)
{
    u8 kDex = 0;
    for(kDex = 0; kDex < Lenth; kDex++)
    {
        if(pData[kDex] < mVal)
        {
            break;
        }
    }
    return (kDex);
}

u8 Calu_TriAnglePlus(u32 Data,u32 Rnd)
{
    u32 mVal = 0;
    u8  Angle = 0;

    Rnd  = Rnd / 10000;
    mVal = Data /Rnd;
    Angle = LookUpTable(aTriAngle,91,(u16)mVal);

    return Angle;
}

u32 XYZ_div3(s32 x,s32 y,s32 z)
{
    u32 dTemp = 0x00;
    
    dTemp = x*x;
    dTemp +=  y*y;
    dTemp +=  z*z;
    dTemp = (u32)sqrt(dTemp);
    
    return (dTemp);
}

void gsensor_study_again(void)
{
    str_mStduy.mFlag  = 0;
    StuMMA.MMAAdjust  = 0xAA;
    StuMMA.ReadMMAVal = 0xAA;
}

void Ffmt_Zthus_Pro(void)
{
    s16 AvgX = 0;
    s16 AvgY = 0;
    s16 AvgZ = 0;
    
    u32 Rnd  = 0;
    u8  Ang_X = 0;
    s8  Ang_Y = 0;
    s8  Ang_Z = 0;
    u8  TempReg = 0;
    s32 sRnd = 0;
    static s16 x_sum = 0;
    static s16 y_sum = 0;
    static s16 z_sum = 0;
    static u8  xyz_Cnt =0;
    static u8  Trig = 0;

    #ifdef RUN_CAR_TEST_LOG
    u8 buffer[1000] = {0};
    u32 len = 0;
    s32 ret = 0;    
    #endif
    
	if(g_Para.OverSw==0)
	{
		return;
	}
	
    if((str_mStduy.mFlag==0)&&((StuMMA.MMAAdjust==0xAA)&&(StuMMA.ReadMMAVal==0xAA)))
    {
        if(str_mStduy.mCount > 3)
        {
            str_mStduy.mCount = 0;
            AvgX = str_mStduy.mxVal;
            AvgY = str_mStduy.myVal;
            AvgZ = str_mStduy.mzVal;
			
            if(((Trig & 0x01) == 0x00) 
                && (Abs(AvgX) < 1200) 
                && (Abs(AvgY) < 1200) 
                && (Abs(AvgZ) < 1200))
            {
                TempReg = 0;
                Trig |= 0x01;
            }
           
            if((Trig & 0x02) == 0x00)
            {
                x_sum += AvgX;
                y_sum += AvgY;
                z_sum += AvgZ;
                xyz_Cnt++;
                if(xyz_Cnt >= 4)
                {
                    str_mStduy.mxAngle = x_sum/xyz_Cnt;
                    str_mStduy.myAngle = y_sum/xyz_Cnt;
                    str_mStduy.mzAngle = z_sum/xyz_Cnt;
                    Trig |= 0x02;
                    xyz_Cnt=0;
                }
            }
            
            if(Trig == 0x03)
            {
                Trig = 0;
                x_sum=0;
                y_sum=0;
                z_sum=0;
                str_mStduy.mFlag = 1;
                StuMMA.MMAAdjust=0;
                StuMMA.ReadMMAVal=0;
				ptmsg("\n自学习完成\n");
            }
        }
    }
    else if(str_mStduy.mFlag==1)
    {
        if(str_mStduy.mCount > 2)//5
        {
            AvgX = str_mStduy.mxVal;
            AvgY = str_mStduy.myVal;
            AvgZ = str_mStduy.mzVal;
            str_mStduy.mCount = 0;
            sRnd =  (s32)(AvgX*str_mStduy.mxAngle); 
            sRnd += (s32)(AvgY*str_mStduy.myAngle);
            sRnd += (s32)(AvgZ*str_mStduy.mzAngle);
            if(sRnd > 0)
            {
                sRnd = Abs32(sRnd);
                Rnd = XYZ_div3(AvgX,AvgY,AvgZ);
                Rnd *= XYZ_div3(str_mStduy.mxAngle,str_mStduy.myAngle,str_mStduy.mzAngle);
                Ang_X = Calu_TriAnglePlus(sRnd,Rnd);
				str_mStduy.ChangAngle=Ang_X;
                if((Ang_X >= g_Para.OverSw))
                {
                    TempReg |= 0x08;
                }
                Ang_Y = Ang_Y;
                Ang_Z = Ang_Z;
            }
            else
            {
				sRnd = Abs32(sRnd);
                Rnd = XYZ_div3(AvgX,AvgY,AvgZ);
                Rnd *= XYZ_div3(str_mStduy.mxAngle,str_mStduy.myAngle,str_mStduy.mzAngle);
                Ang_X = Calu_TriAnglePlus(sRnd,Rnd);
				str_mStduy.ChangAngle=180-Ang_X;
                if((Ang_X >= g_Para.OverSw))
                {
                    TempReg |= 0x08;
                }
                TempReg |= 0x10;
            }
/*            ptmsg("--->tempReg %d Ang_X %d rollover_time %d, mTriCnt %d, shark_alarm %d\r\n", TempReg, Ang_X, app.rollover_time, 
                    str_mStduy.mTriCnt, app.shake_alarm);*/
            if(TempReg)
            {
                str_mStduy.mTriCnt++;
                if(str_mStduy.mTriCnt >= 1)// 10S
                {
                    str_mStduy.mTriCnt = 0;
    			if(app.shake_alarm != 1)
    			{
                        app.shake_alarm = 1;
                        ptmsg("\r\n产生侧翻报警:%d\n",str_mStduy.ChangAngle);
                        g_AppInfo.gsensor.reverseState = 1;
                        ZG_StartTimer(TIMER_ID_ALARM_UPLD, TIM_GEN_1MS, alarm_event_proc);
                        ZG_StartTimer(TIMER_ID_GSENSOR_RESTUDY, TIM_GEN_1SECOND*60, gsensor_study_again);
                        #ifdef RUN_CAR_TEST_LOG
                        len = log_data_pkt(buffer, GSENSOR_ZTHUS_REPORT);
                        
                        if(m_socket.update > 0)
                        {
                            ret = packet_data_send(m_socket.update, buffer, len);
                            if (ret == len)
                            {
                                StatInfo.log_upload++;
                            }
                        }
                        #endif
		        }
                }
            }
            else
            {
                str_mStduy.mTriCnt = 0;
                app.shake_alarm = 0;
            }
            TempReg = 0;
        }
    }
}
void Ffmt_Zthus_Init(void)
{
    Memset(&str_mStduy.mCount,0x00,sizeof(SELFSTDUY_T));
	StuMMA.MMAAdjust=0xAA;
}
#ifdef IMPACT_ALARM_ENABLE
void CheckImpack(void)
{
	#ifdef OptimizeImpact
	u8 ret=0;
	#endif
    if ((g_stuImpAl.cImpaAla != IMPACT_ALARM) && (g_gsensor.max_dif>=g_Para.impact_threshold) \
		&& (g_gsensor.minor_max_dif>=g_Para.impact_threshold)
        &&(g_Para.impact_threshold>0))
    {            
        if(g_stuImpAl.imState == imST_IDLE)
        {
			#ifdef OptimizeImpact
			ret=ImpackSpeedPro(g_GpsSpeed,Estimate);
			if(ret>=3)
			#else
            if(g_GpsSpeed >= 30)
			#endif
            {
                g_stuImpAl.imMmaValue = (g_gsensor.max_dif>g_gsensor.minor_max_dif)?(g_gsensor.max_dif):(g_gsensor.minor_max_dif);
                imNextState(imST_ACCE);
            }
            else
            {
                ptmsg("#####STEP1 speed is low:<%u>",g_GpsSpeed);
            }
        }
        else
        {
            if(g_gsensor.max_dif > g_stuImpAl.imMmaValue)
            {
                g_stuImpAl.imMmaValue = g_gsensor.max_dif;
                ptmsg("######step[%d],imMmaValue[%d]", g_stuImpAl.imState, g_stuImpAl.imMmaValue);
            }
            if(g_gsensor.minor_max_dif > g_stuImpAl.imMmaValue)
            {
                g_stuImpAl.imMmaValue = g_gsensor.minor_max_dif;
                ptmsg("######step[%d],imMmaValue[%d]", g_stuImpAl.imState, g_stuImpAl.imMmaValue);
            }
        } 
    }
    #ifdef IMPACT_ALARM_ENABLE
    if(g_stuImpAl.imState != imST_IDLE)
    {
        g_stuImpAl.impact_det_tim++;
    }
    else
    {
        g_stuImpAl.impact_det_tim = 0;
    }
    #endif
}

ImpackAlaType g_stuImpAl;
void InitImpackAlarm(void)
{
    memset(&g_stuImpAl,0,sizeof(g_stuImpAl));  
}
void imNextState(u8 State)
{
	g_stuImpAl.imState = State;
	g_stuImpAl.imTimer = 0;
}
void MmaImpact_Pro(void)
{
	/// 100ms Tick
    static u8   time100ms=0;
    u8  endofflag = 0;
    #ifdef OptimizeImpact
    static u8 ThrodCnt=0;
    static u8 Tiemcnt=0;
    u32 len = 0;
    s32 ret = 0;    
    #endif

    #ifdef RUN_CAR_TEST_LOG
    u8 buffer[1000] = {0};
    #endif
    
     if(time100ms++<10) 
     {
        return;  
     }
     time100ms = 0;
	g_stuImpAl.imTimer++;
	switch(g_stuImpAl.imState)
	{
		case imST_IDLE:
			g_stuImpAl.imTimer = 0;
		break;

		case imST_ACCE:
		{
		    imNextState(imST_DECT);
			#ifdef OptimizeImpact
			ThrodCnt=0;
			Tiemcnt=0;
			#endif
		    ptmsg("#####STEP1 OK:<0>");
		}
		break;

		case imST_DECT:
		{
			#ifdef OptimizeImpact
			if((g_stuImpAl.imTimer  <g_Para.wImpactDetTime)&&(g_GpsSpeed<5)&&(GPS.Buf[6]=='A'))
			#else
			if((g_stuImpAl.imTimer  >= g_Para.wImpactDetTime) || (g_GpsSpeed<5))
			#endif
			{        
				imNextState(imST_LAST);
				ptmsg("#####STEP 2 OK");
			}
			#ifdef OptimizeImpact
			else if((g_stuImpAl.imTimer >=g_Para.wImpactDetTime)||(GPS.Buf[6]!='A'))
			{
				imNextState(imST_IDLE);
				ptmsg("#####STEP 2 Err Speed:%d",g_GpsSpeed);
			}
			#endif
		}
		break;

		case imST_LAST:
		{
			#ifdef OptimizeImpact
			if((GPS.Buf[6]!='A')||(g_GpsSpeed > 5))  // ADD20190214
			#else
			if(g_GpsSpeed > 5)
			#endif
			{
				endofflag = 3;
				#ifdef OptimizeImpact
				ThrodCnt=0;
				Tiemcnt=0;
				#endif
				ptmsg("#####GpsSpeed=[%d]",g_GpsSpeed);
			}
			else
			{
				///  STEP 3   持续阶段
				if(g_stuImpAl.imTimer >= g_Para.wTmpactLasTime)
				{
				#ifdef OptimizeImpact
					if(ThrodCnt>=10)
					{
                                    ThrodCnt=0;
                                    Tiemcnt=0;
                            #endif
                                    imNextState(imST_IDLE);	
                                    g_stuImpAl.cImpaAla = IMPACT_ALARM;
                                    g_stuImpAl.impact_evt++;
                                    // 立即汇报-------
                                    ptmsg("#####STEP 3 OK");

                                    g_stuImpAl.cImpaAla = 0;
                                    
                                    //设置开启碰撞检测才上报
                                    if (g_Para.crashDetectSW == 1) 
                                    {
                                        g_AppInfo.gsensor.crashState = 1;
                                        ZG_StartTimer(TIMER_ID_ALARM_UPLD, TIM_GEN_1MS, alarm_event_proc);
                                        #ifdef RUN_CAR_TEST_LOG
                                        len = log_data_pkt(buffer, GSENSOR_IMPACT_REPORT);
                                        if(m_socket.update > 0)
                                        {
                                            ret = packet_data_send(m_socket.update, buffer, len);
                                            if (ret == len)
                                            {
                                                StatInfo.log_upload++;
                                            }
                                        }
                                        #endif
                                    }
                            #ifdef OptimizeImpact
					}
					else
					{
						endofflag = 0xFF;
						ThrodCnt=0;
						Tiemcnt=0;
					}
				#endif
				}
				#ifdef OptimizeImpact
				else 
				{
					Tiemcnt++;
					if(Tiemcnt>10) // 一秒钟判断一次数据
					{
						Tiemcnt=0;
						if(g_GpsSpeed==0)
						{
							ThrodCnt++;
						}
					}
				}
				#endif
			}
		}
		break;

		default:
			endofflag = 0xFF;
		break;
	}
	

    if(endofflag)
    {
        ptmsg("#####imError by step%d",endofflag);
        endofflag = 0;
        imNextState(imST_IDLE);
    }
    
}

#endif

#endif




