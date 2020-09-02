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
 * 2020-04-04     lane         ����˼·
 * ...
 */
//2014.07.15�޸İ�
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

static unsigned char g_iicPortAddr = 0;

#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, NPD_ON)
#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, NPD_OFF)

//������Ļ���FM175XX_HardResetҲҪ���Ŷ�Ӧ�޸�
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
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

/*************************************************************/
/*��������	    Read_Reg                                     */
/*���ܣ�	    ���Ĵ�������                                 */
/*���������	reg_add���Ĵ�����ַ                          */
/*����ֵ��	    �Ĵ�����ֵ                                   */
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
/*��������	    Write_Reg									 */
/*���ܣ�	    д�Ĵ�������								 */
/*															 */
/*���������	reg_add���Ĵ�����ַ��reg_value���Ĵ�����ֵ	 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/

unsigned char Write_Reg(unsigned char reg_addr, unsigned char reg_value)
{
    i2c_write_fm17522_reg(reg_addr, reg_value);
	return OK;
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
/*��������	    Read_FIFO									 */
/*���ܣ�	    ��ȡFIFO  									 */
/*															 */
/*���������	length����ȡFIFO���ݳ���					 */
/*���������	*fifo_data��FIFO���ݴ��ָ��				 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/   
unsigned char Read_FIFO(unsigned char length, unsigned char *fifo_data)
{	

    i2c_read_fm17522_fifo(FIFODataReg, fifo_data, length);
    
	return 0;
}

/*************************************************************/
/*��������	    Write_FIFO									 */
/*���ܣ�	    д��FIFO  									 */
/*															 */
/*���������	length����ȡ���ݳ��ȣ�						 */
/*				*fifo_data�����ݴ��ָ��					 */
/*���������												 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/
unsigned char Write_FIFO(unsigned char length, unsigned char *fifo_data)
{
    i2c_write_fm17522_fifo(FIFODataReg, fifo_data, length);
	return 0;
}

/*************************************************************/
/*��������	    Clear_FIFO									 */
/*���ܣ�	    ���FIFO  									 */
/*															 */
/*���������	��											*/
/*���������												 */
/*����ֵ��	    OK											 */
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
/*��������	    Set_BitMask									 */
/*���ܣ�	    ��λ�Ĵ�������								 */
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ�����λ		 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/

unsigned char Set_BitMask(unsigned char reg_addr, unsigned char mask)
{
    unsigned char result;
    result = Read_Reg(reg_addr);
    Write_Reg(reg_addr, result|mask);
    result = OK;
	return result;
}

/*********************************************/
/*��������	    Clear_BitMask   */
/*���ܣ�	    ���λ�Ĵ�������    */
				
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ������λ */
/*����ֵ��	    OK  
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

/*********************************************/
/*��������	    Set_RF  */
/*���ܣ�	    ������Ƶ���    */
				
/*���������	mode����Ƶ���ģʽ  
				0���ر����
				1,����TX1���
				2,����TX2���
				3��TX1��TX2�������TX2Ϊ�������  */
/*����ֵ��	    OK
				ERROR   */
/*********************************************/
unsigned char Set_Rf(unsigned char mode)
{
    unsigned char result;
    
    result = Read_Reg(TxControlReg);
    
    if	((result&0x03) == mode)//chenke, �ж��Ƿ�TX1��TX2�Ѿ����
    {
	    return OK;
    }
    if (mode==0)
	{
	    result=Clear_BitMask(TxControlReg,0x03); //�ر�TX1��TX2���
	}
    if (mode==1)
	{
    	result=Set_BitMask(TxControlReg,0x01); //����TX1���
	}
    if (mode==2)
	{
    	result=Set_BitMask(TxControlReg,0x02); //����TX2���
	}
    if (mode==3)
	{
    	result=Set_BitMask(TxControlReg,0x03); //��TX1��TX2���
	}
    //Delay_100us(1000);//��TX�������Ҫ��ʱ�ȴ������ز��ź��ȶ�
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
/*��������	    Pcd_Comm    */
/*���ܣ�	    ������ͨ��    */
				
/*���������	Command��ͨ�Ų������
				pInData�������������飻
				InLenByte���������������ֽڳ��ȣ�
				pOutData�������������飻
				pOutLenBit���������ݵ�λ����    */
/*����ֵ��	    OK
				ERROR    */
/*********************************************/
unsigned char Pcd_Comm(unsigned char Command,
    unsigned char *pInData,
    unsigned char InLenByte,
    unsigned char *pOutData,
    unsigned int *pOutLenBit)
{
    unsigned char result;
    unsigned char rx_temp;//��ʱ�����ֽڳ���
    unsigned char rx_len;//���������ֽڳ���
    unsigned char lastBits;//��������λ����
    unsigned char irq;
        
    rx_temp = 0;
    rx_len = 0;
    lastBits = 0;
    
	Clear_FIFO();
    
   	Write_Reg(CommandReg,Idle);
//    rx_temp = Read_Reg(CommandReg);
    
   	Write_Reg(WaterLevelReg,0x20);//����FIFOLevel=32�ֽ�
//   	rx_temp = Read_Reg(WaterLevelReg);
    
	Write_Reg(ComIrqReg,0x7F);//���IRQ��־
//	rx_temp = Read_Reg(ComIrqReg);

    rx_temp = 0;
    
 	if(Command == MFAuthent)
	{
		Write_FIFO(InLenByte,pInData);//������֤��Կ	
		Set_BitMask(BitFramingReg,0x80);//��������
	}
    Set_BitMask(TModeReg,0x80);//�Զ�������ʱ��

 	Write_Reg(CommandReg,Command);

    //�Լ�������ʱ�жϣ�100ms��ʱ
    i2c_comm_timeout_cnt = 400;//200ʱ����볬ʱ����
	while(1)//ѭ���ж��жϱ�ʶ
    {
        if(!i2c_comm_timeout_cnt)
        {
            //�Լ��жϳ�ʱ�����ش���
            result = ERROR;
            return result;
        }
        
        irq = Read_Reg(ComIrqReg);//��ѯ�жϱ�־		

        if(irq&0x01)	//TimerIRq  ��ʱ��ʱ���þ�
        {
            result=ERROR;		
            break;
        }
        if(Command == MFAuthent)
        {
            if(irq & 0x10)	//IdelIRq  command�Ĵ���Ϊ���У�ָ��������
            {
                result=OK;
                break;
            }
        }
        if(Command == Transmit)	
        {
            if((irq & 0x04) && (InLenByte > 0))	//LoAlertIrq+�����ֽ�������0
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
                Set_BitMask(BitFramingReg,0x80);	//��������

                Write_Reg(ComIrqReg,0x04);	//���LoAlertIrq
            }	

            if((irq & 0x40) && (InLenByte == 0))	//TxIRq
            {
                result=OK;
                break;
            }
        }

        if(Command == Transceive)
        {
            if((irq & 0x04) && (InLenByte > 0))	//LoAlertIrq + �����ֽ�������0
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
                Set_BitMask(BitFramingReg,0x80);//��������
                Write_Reg(ComIrqReg,0x04);//���LoAlertIrq
            }
            if(irq & 0x08)	//HiAlertIRq
            {
                rx_temp = Read_Reg(FIFOLevelReg);

                #define FM175XX_GET_RX_DATA_LEVEL 32
                if((irq & 0x40) && (InLenByte == 0) && (rx_temp > FM175XX_GET_RX_DATA_LEVEL))//TxIRq	+ �����ͳ���Ϊ0 + FIFO���ȴ���32
                {
                    Read_FIFO(FM175XX_GET_RX_DATA_LEVEL, pOutData + rx_len); //����FIFO����
                    rx_len = rx_len + FM175XX_GET_RX_DATA_LEVEL;
                    Write_Reg(ComIrqReg,0x08);	//��� HiAlertIRq
                }
            }
            if((irq & 0x20) && (InLenByte == 0))	//RxIRq=1
            {
                result = OK;
                break;
            }
        }
    }
//    if (Read_Reg(ErrorReg)&0x0F)	//Mifare��Ƭ��д����ʱ�᷵��4��BIT,��Ҫ���δ�����
//        {
// 			result = ERROR;
//        }
//        else
    {   
        if (Command == Transceive)
        {
            rx_temp = Read_Reg(FIFOLevelReg);
            Read_FIFO(rx_temp, pOutData + rx_len); //����FIFO����
            rx_len= rx_len + rx_temp;//���ճ����ۼ�
            
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
    Clear_BitMask(BitFramingReg,0x80);//�رշ���
    return result;
}

/*********************************************/
/*��������	    Pcd_SetTimer    */
/*���ܣ�	    ���ý�����ʱ    */
/*���������	delaytime����ʱʱ�䣨��λΪ���룩    */
/*����ֵ��	    OK    */
/*********************************************/
unsigned char Pcd_SetTimer(unsigned long delaytime)//�趨��ʱʱ�䣨ms��
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
/*��������	    Pcd_ConfigISOType    */
/*���ܣ�	    ���ò���Э��    */
/*���������	type 0��ISO14443AЭ�飻    */
/*					 1��ISO14443BЭ�飻    */
/*����ֵ��	    OK    */
/*********************************************/
unsigned char Pcd_ConfigISOType(unsigned char type)
{
    if (type == 0)                     //ISO14443_A
    { 
        Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ// chenke, FM17522 ��bit4��RFTԤ��       
        Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 ����100%ASK��Ч        
        Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A// chenke, TX CRC��Ч, TXSPEED 106Kbit        
        Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A// chenke, RX CRC��Ч, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
    }
    if (type == 1)                     //ISO14443_B
    { 
        Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ        
        Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE B        
        Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE B        
        Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 ����ON�絼        
        Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 ����OFF�絼        
        Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 ����100%ASK��Ч        
    }
    return OK;
}

/*********************************************/
/*��������	    FM175XX_SoftReset    */
/*���ܣ�	    ����λ����    */			
/*���������	    */
/*����ֵ��	    OK
				ERROR    */
/*********************************************/
unsigned char  FM175XX_SoftReset(void)
{	
    unsigned char result;
    Write_Reg(CommandReg,SoftReset);//chenke, ��λFM17522
    result = Set_BitMask(ControlReg,0x10);//17520��ʼֵ����//chenke, FM17522 RFTλ��1
    result = OK;
    return result;
}

unsigned char FM175XX_HardReset(void)
{	
	if(g_iicPortAddr == 0)
	{
		//����NPD��
		FM17522_NPD_LOW;
		FM17522_Delayms(10);
		//����NPD��
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	else if(g_iicPortAddr == 1)
	{
		//����NPD��
		FM17522_NPD_LOW;
		FM17522_Delayms(1);
		//����NPD��
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	return ERROR;
}

unsigned char FM175XX_switchPort(uint8_t port)
{
	g_iicPortAddr = port;
	if (port == 0)
	{
		//����NPD��
		FM17522_NPD_LOW;
		FM17522_Delayms(1);
		//����NPD��
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	else if (port == 1)
	{
		//����NPD��
		FM17522_NPD_LOW;
		FM17522_Delayms(1);
		//����NPD��
		FM17522_NPD_HIGHT;
		FM17522_Delayms(10);
		return OK;
	}
	return ERROR;
}

/*********************************************/
/*��������	    FM175XX_SoftPowerdown    */
/*���ܣ�	    �����͹��Ĳ���    */
/*���������	    */
/*����ֵ��	    OK������͹���ģʽ��    */
/*				ERROR���˳��͹���ģʽ��    */
/*********************************************/
unsigned char FM175XX_SoftPowerdown(void)
{
//    unsigned char result;
//    result = 
	Read_Reg(CommandReg);
    
	//if (gl_fm_FMSP_param.result&0x10)
	//{
		//Clear_BitMask(CommandReg,0x10);//�˳��͹���ģʽ
    //    CLEAR_BITMASK(CommandReg,0x10);
    //    PT_INIT(pl_pt);
	//	return ERROR;
	//}
	//else
    {   
        Set_BitMask(CommandReg,0x10);//����͹���ģʽ
    }
    return OK;
}

/*********************************************/
/*��������	    FM175XX_HardPowerdown    */
/*���ܣ�	    Ӳ���͹��Ĳ���    */
/*���������	    */
/*����ֵ��	    OK������͹���ģʽ��    */
/*				ERROR���˳��͹���ģʽ��    */
/*********************************************/
unsigned char FM175XX_HardPowerdown(void)
{	
	if(g_iicPortAddr == 0)
	{
		//����NPD��
		FM17522_NPD_LOW;
		return OK;
	}
	else if(g_iicPortAddr == 1)
	{
		//����NPD��
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
	nfc_ms_handler = rt_timer_create("nfc_ms_timer", nfc_ms_timer_cb, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);
    if (nfc_ms_handler != RT_NULL) 
        rt_timer_start(nfc_ms_handler);
}



#endif