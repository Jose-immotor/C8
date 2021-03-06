#if 1
/*
 * File      : fm175xx.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-15     kim 		   the first version
 * 2020-04-04     lane         完善思路
 * ...
 */
//2014.07.15修改版
/*************************************************************/
#define FM17522_C
#include "fm175xx.h"
#include "drv_i2c.h"
#include <rtthread.h>
#include "drv_gpio.h"

#define ERROR		1
#define	OK			0

#define NPD_ON		1
#define	NPD_OFF		0

static DrvIo* g_pNfcNpdAIO = Null;
static DrvIo* g_pNfcPwrOffIO = Null;

static unsigned char g_iicPortAddr = 0;

#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, NPD_ON)
#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, NPD_OFF)

//改这里的话，FM175XX_HardReset也要跟着对应修改
const unsigned char gl_FM17522_Slave_Addr[6] = {FM17522_I2C_ADDR, FM17522_I2C_ADDR1, 0x29, 0x0f, 0x32, 0x33};

unsigned char Fm17522_get_slave_addr(void)
{
    return gl_FM17522_Slave_Addr[g_iicPortAddr];
}


unsigned char i2c_read_fm17522_reg(unsigned char reg_addr, unsigned char *value)
{    
	unsigned char dev_addr;
	
	dev_addr = Fm17522_get_slave_addr();

    if (gd32_i2c_read(NFC_I2C,dev_addr,value,reg_addr,1))
    {
        return OK;
    }
    else
    {
        return ERROR;
    }
}

rt_err_t i2c_write_fm17522_reg(rt_uint8_t reg_addr, rt_uint8_t value)
{
	unsigned char dev_addr;
	unsigned char data;
	
	data = value;
	dev_addr = Fm17522_get_slave_addr();
    if (gd32_i2c_write(NFC_I2C,dev_addr, &data,reg_addr,1))
    {
        return RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }

}

/*************************************************************/
/*函数名：	    Read_Reg                                     */
/*功能：	    读寄存器操作                                 */
/*输入参数：	reg_add，寄存器地址                          */
/*返回值：	    寄存器数值                                   */
/*************************************************************/
unsigned char Read_Reg(unsigned char reg_addr)
{
    unsigned char reg_value;

    i2c_read_fm17522_reg(reg_addr, &reg_value);
            
    return reg_value;
}

rt_err_t i2c_read_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len)
{    
//    rt_uint8_t vl_buf[255];
	unsigned char dev_addr;
	
    dev_addr = Fm17522_get_slave_addr();
    if (gd32_i2c_read(NFC_I2C,dev_addr,fifo_buf, fifo_reg_addr, fifo_buf_len) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

/*************************************************************/
/*函数名：	    Write_Reg									 */
/*功能：	    写寄存器操作								 */
/*															 */
/*输入参数：	reg_add，寄存器地址；reg_value，寄存器数值	 */
/*返回值：	    OK											 */
/*				ERROR										 */
/*************************************************************/

unsigned char Write_Reg(unsigned char reg_addr, unsigned char reg_value)
{
	return i2c_write_fm17522_reg(reg_addr, reg_value);
}

rt_err_t i2c_write_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len)
{
    rt_uint8_t vl_buf[255];
	unsigned char dev_addr;
	
    dev_addr = Fm17522_get_slave_addr();
    if(fifo_buf_len > 255)
    {
        rt_memcpy(&vl_buf[0], fifo_buf, 255);
        fifo_buf_len = 255;
    }
    else
    {
        rt_memcpy(&vl_buf[0], fifo_buf, fifo_buf_len);
    }

    if (gd32_i2c_write(NFC_I2C,dev_addr,vl_buf,fifo_reg_addr,fifo_buf_len))
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

/*************************************************************/
/*函数名：	    Read_FIFO									 */
/*功能：	    读取FIFO  									 */
/*															 */
/*输入参数：	length，读取FIFO数据长度					 */
/*输出参数：	*fifo_data，FIFO数据存放指针				 */
/*返回值：	    											 */
/*															 */
/*************************************************************/   
unsigned char Read_FIFO(unsigned char length, unsigned char *fifo_data)
{	

    i2c_read_fm17522_fifo(FIFODataReg, fifo_data, length);
    
	return 0;
}

/*************************************************************/
/*函数名：	    Write_FIFO									 */
/*功能：	    写入FIFO  									 */
/*															 */
/*输入参数：	length，读取数据长度；						 */
/*				*fifo_data，数据存放指针					 */
/*输出参数：												 */
/*返回值：	    											 */
/*															 */
/*************************************************************/
unsigned char Write_FIFO(unsigned char length, unsigned char *fifo_data)
{
    i2c_write_fm17522_fifo(FIFODataReg, fifo_data, length);
	return 0;
}

/*************************************************************/
/*函数名：	    Clear_FIFO									 */
/*功能：	    清空FIFO  									 */
/*															 */
/*输入参数：	无											*/
/*输出参数：												 */
/*返回值：	    OK											 */
/*				ERROR										 */
/*************************************************************/
unsigned char Clear_FIFO(void)
{
    unsigned char result;
    //gl_fm_SBM_param.reg_add = FIFOLevelReg;
    //gl_fm_SBM_param.mask = 0x80;
    //FUNC_WAIT_WITH_RETURN(Set_BitMask, gl_fm_SBM_param);
    Set_BitMask(FIFOLevelReg,0x80);

    //gl_fm_RG_param.reg_add = FIFOLevelReg;
    //FUNC_WAIT_WITH_RESULT_AND_RETURN(Read_Reg,gl_fm_RG_param,gl_fm_CFIFO_param.result);
    result = Read_Reg(FIFOLevelReg);
    
    if (result == 0)
    {
        return OK;
    }
    else
    {
        return ERROR;
    }
}

/*************************************************************/
/*函数名：	    Set_BitMask									 */
/*功能：	    置位寄存器操作								 */
/*输入参数：	reg_add，寄存器地址；mask，寄存器置位		 */
/*返回值：	    OK											 */
/*				ERROR										 */
/*************************************************************/

unsigned char Set_BitMask(unsigned char reg_addr, unsigned char mask)
{
    unsigned char result;
    result = Read_Reg(reg_addr);
    result = Write_Reg(reg_addr, result|mask);
//    result = OK;
	return result;
}

/*********************************************/
/*函数名：	    Clear_BitMask   */
/*功能：	    清除位寄存器操作    */
				
/*输入参数：	reg_add，寄存器地址；mask，寄存器清除位 */
/*返回值：	    OK  
				ERROR   */
/*********************************************/
unsigned char Clear_BitMask(unsigned char reg_addr, unsigned char mask)
{
    unsigned char result;
    result = Read_Reg(reg_addr);
    Write_Reg(reg_addr, result & (~mask));
    result = OK;
	return result;
}
#if 1
/*********************************************/
/*函数名：	    Set_RF  */
/*功能：	    设置射频输出    */
				
/*输入参数：	mode，射频输出模式  
				0，关闭输出
				1,仅打开TX1输出
				2,仅打开TX2输出
				3，TX1，TX2打开输出，TX2为反向输出  */
/*返回值：	    OK
				ERROR   */
/*********************************************/
unsigned char Set_Rf(unsigned char mode)
{
    unsigned char result;
    
    result = Read_Reg(TxControlReg);
    
    if	((result&0x03) == mode)//chenke, 判断是否TX1和TX2已经输出
    {
	    return OK;
    }
    if (mode==0)
	{
	    result=Clear_BitMask(TxControlReg,0x03); //关闭TX1，TX2输出
	}
    if (mode==1)
	{
    	result=Set_BitMask(TxControlReg,0x01); //仅打开TX1输出
	}
    if (mode==2)
	{
    	result=Set_BitMask(TxControlReg,0x02); //仅打开TX2输出
	}
    if (mode==3)
	{
    	result=Set_BitMask(TxControlReg,0x03); //打开TX1，TX2输出
	}
    //Delay_100us(1000);//打开TX输出后需要延时等待天线载波信号稳定
    FM17522_Delayms(1);

    return result;
}
uint16_t i2c_comm_timeout_cnt=0;
void Pcd_Comm_timer_cb(void)
{
    if(i2c_comm_timeout_cnt)
        i2c_comm_timeout_cnt--;
}
/*********************************************/
/*函数名：	    Pcd_Comm    */
/*功能：	    读卡器通信    */
				
/*输入参数：	Command，通信操作命令；
				pInData，发送数据数组；
				InLenByte，发送数据数组字节长度；
				pOutData，接收数据数组；
				pOutLenBit，接收数据的位长度    */
/*返回值：	    OK
				ERROR    */
/*********************************************/
unsigned char Pcd_Comm(unsigned char Command,
    unsigned char *pInData,
    unsigned char InLenByte,
    unsigned char *pOutData,
    unsigned int *pOutLenBit)
{
    unsigned char result;
    unsigned char rx_temp;//临时数据字节长度
    unsigned char rx_len;//接收数据字节长度
    unsigned char lastBits;//接收数据位长度
    unsigned char irq;
        
    rx_temp = 0;
    rx_len = 0;
    lastBits = 0;
    
	Clear_FIFO();
    
   	Write_Reg(CommandReg,Idle);
//    rx_temp = Read_Reg(CommandReg);
    
   	Write_Reg(WaterLevelReg,0x20);//设置FIFOLevel=32字节
//   	rx_temp = Read_Reg(WaterLevelReg);
    
	Write_Reg(ComIrqReg,0x7F);//清除IRQ标志
//	rx_temp = Read_Reg(ComIrqReg);

    rx_temp = 0;
    
 	if(Command == MFAuthent)
	{
		Write_FIFO(InLenByte,pInData);//填入认证密钥	
		Set_BitMask(BitFramingReg,0x80);//启动发送
	}
    Set_BitMask(TModeReg,0x80);//自动启动定时器

 	Write_Reg(CommandReg,Command);

    //自己再做超时判断，100ms超时
    i2c_comm_timeout_cnt = 400;//200时会进入超时错误
	while(1)//循环判断中断标识
    {
        if(!i2c_comm_timeout_cnt)
        {
            //自己判断超时，返回错误
            result = ERROR;
            return result;
        }
        
        irq = Read_Reg(ComIrqReg);//查询中断标志		

        if(irq&0x01)	//TimerIRq  定时器时间用尽
        {
            result=ERROR;		
            break;
        }
        if(Command == MFAuthent)
        {
            if(irq & 0x10)	//IdelIRq  command寄存器为空闲，指令操作完成
            {
                result=OK;
                break;
            }
        }
        if(Command == Transmit)	
        {
            if((irq & 0x04) && (InLenByte > 0))	//LoAlertIrq+发送字节数大于0
            {
                if (InLenByte < 32)
                {
                    Write_FIFO(InLenByte,pInData);	
                    InLenByte=0;
                }
                else
                {
                    Write_FIFO(32,pInData);

                    InLenByte = InLenByte - 32;
                    pInData = pInData + 32;
                }
                Set_BitMask(BitFramingReg,0x80);	//启动发送

                Write_Reg(ComIrqReg,0x04);	//清除LoAlertIrq
            }	

            if((irq & 0x40) && (InLenByte == 0))	//TxIRq
            {
                result=OK;
                break;
            }
        }

        if(Command == Transceive)
        {
            if((irq & 0x04) && (InLenByte > 0))	//LoAlertIrq + 发送字节数大于0
            {	
                if (InLenByte > 32)
                {
                    Write_FIFO(32,pInData);
                    InLenByte = InLenByte - 32;
                    pInData = pInData + 32;
                }
                else
                {
                    Write_FIFO(InLenByte,pInData);
                    InLenByte = 0;
                }
                Set_BitMask(BitFramingReg,0x80);//启动发送
                Write_Reg(ComIrqReg,0x04);//清除LoAlertIrq
            }
            if(irq & 0x08)	//HiAlertIRq
            {
                rx_temp = Read_Reg(FIFOLevelReg);

                #define FM175XX_GET_RX_DATA_LEVEL 32
                if((irq & 0x40) && (InLenByte == 0) && (rx_temp > FM175XX_GET_RX_DATA_LEVEL))//TxIRq	+ 待发送长度为0 + FIFO长度大于32
                {
                    Read_FIFO(FM175XX_GET_RX_DATA_LEVEL, pOutData + rx_len); //读出FIFO内容
                    rx_len = rx_len + FM175XX_GET_RX_DATA_LEVEL;
                    Write_Reg(ComIrqReg,0x08);	//清除 HiAlertIRq
                }
            }
            if((irq & 0x20) && (InLenByte == 0))	//RxIRq=1
            {
                result = OK;
                break;
            }
        }
    }
//    if (Read_Reg(ErrorReg)&0x0F)	//Mifare卡片的写操作时会返回4个BIT,需要屏蔽错误检测
//        {
// 			result = ERROR;
//        }
//        else
    {   
        if (Command == Transceive)
        {
            rx_temp = Read_Reg(FIFOLevelReg);
            Read_FIFO(rx_temp, pOutData + rx_len); //读出FIFO内容
            rx_len= rx_len + rx_temp;//接收长度累加
            
            //lastBits = Read_Reg(ControlReg) & 0x07;
            lastBits = Read_Reg(ControlReg);
            lastBits &=  0x07;

            if (lastBits)
                *pOutLenBit = (rx_len - 1) * (unsigned int)8 + lastBits;  
            else
                *pOutLenBit = rx_len * (unsigned int)8;   
        }
    }
    Set_BitMask(ControlReg,0x80);           // stop timer now
    Write_Reg(CommandReg,Idle); 
    Clear_BitMask(BitFramingReg,0x80);//关闭发送
    return result;
}

//static uint16_t RX_POINT = 0,TX_POINT=0;

//void nfc_intisr_cb(unsigned char *pInData, unsigned char InLenByte,
//					unsigned char *pOutData, unsigned int *pOutLenBit)
//{
//    unsigned char irq;
//    uint16_t cnt = 0;
//	unsigned char rx_temp;//临时数据字节长度
//    unsigned char rx_len;//接收数据字节长度
//    
//	irq = Read_Reg(ComIrqReg);//查询中断标志

//	if((irq & 0x04)&&(InLenByte > TX_POINT))//渐空LoAlert&&有发送请求
//	{
//		cnt = InLenByte-TX_POINT;
//		if (cnt > 32)
//		{
//			Write_FIFO(32,pInData+TX_POINT);
//			TX_POINT += 32;
//		}
//		else
//		{
//			Write_FIFO(InLenByte,pInData+TX_POINT);
//			TX_POINT = TX_POINT + InLenByte;
//		}
//		Set_BitMask(BitFramingReg,0x80);//启动发送
//		Write_Reg(ComIrqReg,0x04);//清除LoAlertIrq
//	}
//	else if(irq & 0x08)//渐满HiAlert
//	{
//	    rx_temp = Read_Reg(FIFOLevelReg);
//		Read_FIFO(rx_temp, pOutData + RX_POINT); //读出FIFO内容
//		RX_POINT = RX_POINT + rx_temp;
//		Write_Reg(ComIrqReg,0x08);	//清除 HiAlertIRq
//	}
//	else if(irq & 0x20)//RxIRq
//	{
//	    rx_temp = Read_Reg(FIFOLevelReg);
//		Read_FIFO(rx_temp, pOutData + RX_POINT); //读出FIFO内容
//		RX_POINT = RX_POINT + rx_temp;
//		Write_Reg(ComIrqReg,0x08);	//清除 HiAlertIRq
////		*pOutLenBit = RX_POINT;
//	}        
//}

//unsigned char nfc_frame_tx(unsigned char *pInData, unsigned char InLenByte)
//{
//	TX_POINT = 0;
//	Write_Reg(ComIrqReg,0x7F);//清除IRQ标志
//	Clear_FIFO();
//    if(InLenByte>=32)
//    {
//        Write_FIFO(32,pInData);
//        TX_POINT+=32;
//    }
//    else
//    {
//        Write_FIFO(InLenByte,pInData);
//        TX_POINT+=InLenByte;
//    }
//    Set_BitMask(BitFramingReg,0x80);//启动发送
//    return OK;
//}


/*********************************************/
/*函数名：	    Pcd_SetTimer    */
/*功能：	    设置接收延时    */
/*输入参数：	delaytime，延时时间（单位为毫秒）    */
/*返回值：	    OK    */
/*********************************************/
unsigned char Pcd_SetTimer(unsigned long delaytime)//设定超时时间（ms）
{
	unsigned long TimeReload;
	unsigned int Prescaler;

	Prescaler = 0;
	TimeReload = 0;
	while(Prescaler < 0xfff)
	{
		TimeReload = ((delaytime*(long)13560) - 1) / (Prescaler*2 + 1);
		if( TimeReload < 0xffff)
			break;
		Prescaler++;
	}
	TimeReload = TimeReload&0xFFFF;
	Set_BitMask(TModeReg, Prescaler>>8);
	Write_Reg(TPrescalerReg, Prescaler&0xFF);
	Write_Reg(TReloadMSBReg, TimeReload>>8);
	Write_Reg(TReloadLSBReg,TimeReload&0xFF);
	return OK;
}

/*********************************************/
/*函数名：	    Pcd_ConfigISOType    */
/*功能：	    设置操作协议    */
/*输入参数：	type 0，ISO14443A协议；    */
/*					 1，ISO14443B协议；    */
/*返回值：	    OK    */
/*********************************************/
unsigned char Pcd_ConfigISOType(unsigned char type)
{
    if (type == 0)                     //ISO14443_A
    { 
        Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式// chenke, FM17522 的bit4是RFT预留       
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 设置100%ASK有效        
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A// chenke, TX CRC无效, TXSPEED 106Kbit        
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A// chenke, RX CRC无效, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
    }
    if (type == 1)                     //ISO14443_B
    { 
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式        
        Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 设置TX CRC有效，TX FRAMING =TYPE B        
        Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 设置RX CRC有效，RX FRAMING =TYPE B        
        Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 设置ON电导        
        Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 设置OFF电导        
        Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 设置100%ASK无效        
    }
    return OK;
}

/*********************************************/
/*函数名：	    FM175XX_SoftReset    */
/*功能：	    软复位操作    */			
/*输入参数：	    */
/*返回值：	    OK
				ERROR    */
/*********************************************/
unsigned char  FM175XX_SoftReset(void)
{	
    unsigned char result;
    Write_Reg(CommandReg,SoftReset);//chenke, 复位FM17522
    result = Set_BitMask(ControlReg,0x10);//17520初始值配置//chenke, FM17522 RFT位置1
//    result = OK;
    return result;
}

unsigned char FM175XX_HardReset(void)
{	
	if(g_iicPortAddr == 0)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		FM17522_Delayms(10);
		//拉高NPD脚
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	else if(g_iicPortAddr == 1)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		FM17522_Delayms(1);
		//拉高NPD脚
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	return ERROR;
}

unsigned char FM175XX_switchPort(uint8_t port)
{
	g_iicPortAddr = 0;//port;
	if (port == 0)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		FM17522_Delayms(10);
		//拉高NPD脚
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	else if (port == 1)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		FM17522_Delayms(10);
		//拉高NPD脚
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	return ERROR;
}

/*********************************************/
/*函数名：	    FM175XX_SoftPowerdown    */
/*功能：	    软件低功耗操作    */
/*输入参数：	    */
/*返回值：	    OK，进入低功耗模式；    */
/*				ERROR，退出低功耗模式；    */
/*********************************************/
unsigned char FM175XX_SoftPowerdown(void)
{
//    unsigned char result;
//    result = 
	Read_Reg(CommandReg);
    
	//if (gl_fm_FMSP_param.result&0x10)
	//{
		//Clear_BitMask(CommandReg,0x10);//退出低功耗模式
    //    CLEAR_BITMASK(CommandReg,0x10);
    //    PT_INIT(pl_pt);
	//	return ERROR;
	//}
	//else
    {   
        Set_BitMask(CommandReg,0x10);//进入低功耗模式
    }
    return OK;
}

/*********************************************/
/*函数名：	    FM175XX_HardPowerdown    */
/*功能：	    硬件低功耗操作    */
/*输入参数：	    */
/*返回值：	    OK，进入低功耗模式；    */
/*				ERROR，退出低功耗模式；    */
/*********************************************/
unsigned char FM175XX_HardPowerdown(void)
{	
	if(g_iicPortAddr == 0)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		return OK;
	}
	else if(g_iicPortAddr == 1)
	{
		//拉低NPD脚
		FM17522_NPD_LOW;
		return OK;
	}
	return ERROR;
}

void FM17522_Delayms(unsigned int delayms)
{
    rt_thread_mdelay(delayms);
	return ;
}

static void nfc_ms_timer_cb(void* parameter)
{
	Pcd_Comm_timer_cb();//NFC timeout
}

rt_timer_t nfc_ms_handler;
void FM17522_Init(void)
{
    rt_hw_i2c_init(NFC_I2C);
	g_pNfcNpdAIO = IO_Get(IO_NFC_NPD_A);
	g_pNfcPwrOffIO=IO_Get(IO_NFC_PWR_OFF);
	PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, False);
	nfc_ms_handler = rt_timer_create("nfc_ms_timer", nfc_ms_timer_cb, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);
    if (nfc_ms_handler != RT_NULL) 
        rt_timer_start(nfc_ms_handler);
}

#endif

#endif
