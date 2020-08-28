/*
 * File      : drv_i2c.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-06-05     tanek        first implementation.
 * 2018-04-19     misonyo      Porting for gd32f30x
 * 2019-12-16     lane         Porting for gd32f403,central
 */

#include "drv_i2c.h"
#include <rtthread.h>
#include "gd32f403.h"

static void GPIO_Configuration_I2C(uint32_t i2c_periph)
{
    uint32_t GPIO_SDA;
    uint32_t GPIO_SCL;
    uint32_t GPIO_Pin_SDA, GPIO_Pin_SCL;

    if(i2c_periph == I2C0)
    {    
        rcu_periph_reset_enable(RCU_I2C0RST); 
        rcu_periph_reset_disable(RCU_I2C0RST); 
        /* enable GPIOB clock */
        rcu_periph_clock_enable(RCU_GPIOB);
        /* enable i2c_periph clock */
        rcu_periph_clock_enable(RCU_I2C0);
        GPIO_SCL = GPIOB;
        GPIO_Pin_SCL = GPIO_PIN_6;
        GPIO_SDA = GPIOB;
        GPIO_Pin_SDA = GPIO_PIN_7;
    }
    else if(i2c_periph == I2C1) 
    {
        rcu_periph_reset_enable(RCU_I2C1RST); 
        rcu_periph_reset_disable(RCU_I2C1RST); 
        /* enable GPIOB clock */
        rcu_periph_clock_enable(RCU_GPIOB);
        /* enable I2C1 clock */
        rcu_periph_clock_enable(RCU_I2C1);
        GPIO_SCL = GPIOB;
        GPIO_Pin_SCL = GPIO_PIN_10;
        GPIO_SDA = GPIOB;
        GPIO_Pin_SDA = GPIO_PIN_11;
    }

    gpio_init(GPIO_SCL, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_Pin_SCL);
    gpio_init(GPIO_SDA, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_Pin_SDA);
}
void I2C_init(uint32_t i2c_periph)
{

    GPIO_Configuration_I2C(i2c_periph);
	i2c_deinit(i2c_periph);
    /* I2C clock configure */
    i2c_clock_config(i2c_periph, 100000, I2C_DTCY_2);
    /* enable i2c_periph */
    i2c_enable(i2c_periph);
    /* enable acknowledge */
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
}
void Delay_I2C(uint32_t i)
{
//	rt_thread_delay(i);
//	rt_thread_mdelay(i);
    while(i--);
}
void Resume_IIC(uint32_t Timeout, uint32_t i2c_periph)
{
	#if 1
    uint32_t GPIO_SDA;
    uint32_t GPIO_SCL;
    uint32_t GPIO_Pin_SDA, GPIO_Pin_SCL;

//    if(i2c_periph == I2C0)
//    {    
//        /* enable GPIOB clock */
//        rcu_periph_clock_enable(RCU_GPIOB);
//        /* disable i2c_periph clock */
//        rcu_periph_clock_disable(RCU_I2C0);
//        GPIO_SCL = GPIOB;
//        GPIO_Pin_SCL = GPIO_PIN_6;
//        GPIO_SDA = GPIOB;
//        GPIO_Pin_SDA = GPIO_PIN_7;
//    }
//    else if(i2c_periph == I2C1)
    {
        /* enable GPIOB clock */
        rcu_periph_clock_enable(RCU_GPIOB);
        /* disable I2C1 clock */
        rcu_periph_clock_disable(RCU_I2C1);
        GPIO_SCL = GPIOB;
        GPIO_Pin_SCL = GPIO_PIN_10;
        GPIO_SDA = GPIOB;
        GPIO_Pin_SDA = GPIO_PIN_11;
    }
    
    do{
        gpio_init(GPIO_SCL, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, GPIO_Pin_SCL);
        gpio_init(GPIO_SDA, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, GPIO_Pin_SDA);
        gpio_bit_reset(GPIO_SCL, GPIO_Pin_SCL);
        rt_thread_delay(20);
        gpio_bit_reset(GPIO_SDA, GPIO_Pin_SDA);
        rt_thread_delay(20);
        gpio_bit_set(GPIO_SCL, GPIO_Pin_SCL);
        rt_thread_delay(20);
        gpio_bit_set(GPIO_SDA, GPIO_Pin_SDA);
        rt_thread_delay(20);
		Timeout--;
        if( Timeout == 0)
		{
            Timeout = 0;
			I2C_init(i2c_periph);
			return;
		}
    }while((!gpio_input_bit_get(GPIO_SDA, GPIO_Pin_SDA))||(!gpio_input_bit_get(GPIO_SCL, GPIO_Pin_SCL)));
    
    I2C_init(i2c_periph);
	#endif
}
int gd32_i2c_read(uint32_t i2c_periph, uint8_t slave_address, uint8_t* p_buffer,uint8_t read_address,uint8_t data_byte)
{
//    rt_uint32_t vl_delay_cnt;
	uint32_t I2C_Timeout = I2C_SHORT_TIMEOUT; 

    /* wait until I2C bus is idle */
    while((i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY)))
    {
        if((I2C_Timeout--) == 0) 
        { 
			Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	    
	if(2 == data_byte)
    {
        i2c_ackpos_config(i2c_periph,I2C_ACKPOS_NEXT);
    }
	
    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);
	I2C_Timeout = I2C_SHORT_TIMEOUT; 
    while(!(i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND)))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        }
    }
	/* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, slave_address<<1, I2C_TRANSMITTER);
	/* wait until ADDSEND bit is set */
	I2C_Timeout = I2C_SHORT_TIMEOUT; 
    while(!(i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND)))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        }
    }
	/* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);
	
    I2C_Timeout = I2C_SHORT_TIMEOUT;     
    /* wait until the transmit data buffer is empty */
    while(SET != i2c_flag_get( i2c_periph , I2C_FLAG_TBE))
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	
    /* send the slave's internal address to write to */
    i2c_data_transmit(i2c_periph, read_address);  	
	
	I2C_Timeout = I2C_SHORT_TIMEOUT;     
    /* wait until BTC bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	
    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);	
	
    I2C_Timeout = I2C_SHORT_TIMEOUT;    
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }  
    /* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, slave_address<<1, I2C_RECEIVER);	
	
    if(data_byte < 3)
    {
        /* disable acknowledge */
        i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
    }
	
	I2C_Timeout = I2C_SHORT_TIMEOUT;      
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }   
	/* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

    if(1 == data_byte)
    {
        i2c_stop_on_bus(i2c_periph);
    }
    
    /* while there is data to be read */
    while(data_byte){
        if(3 == data_byte)
        {
            I2C_Timeout = I2C_SHORT_TIMEOUT;  
            /* wait until BTC bit is set */
            while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
            { 
                if((I2C_Timeout--) == 0) 
                { 
                    Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
                    return RT_ERROR; 
                } 
            } 
    
            /* disable acknowledge */
            i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
        }
        
        if(2 == data_byte)
        {
            I2C_Timeout = I2C_SHORT_TIMEOUT; 
            /* wait until BTC bit is set */
            while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
            { 
                if((I2C_Timeout--) == 0) 
                { 
                    Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
                    return RT_ERROR; 
                } 
            } 
            
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(i2c_periph);
            I2C_Timeout = I2C_SHORT_TIMEOUT;
            /* wait until stop condition generate */ 
            while(I2C_CTL0(i2c_periph)&0x0200)
            { 
                if((I2C_Timeout--) == 0) 
                { 
                    Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
                    return RT_ERROR; 
                } 
            } 
        }
        
        /* wait until the RBNE bit is set and clear it */
        if(i2c_flag_get(i2c_periph, I2C_FLAG_RBNE))
        {
            /* read a byte from the slave */
            *p_buffer = i2c_data_receive(i2c_periph);
            
            /* point to the next location where the byte read will be saved */
            p_buffer++; 
            
            /* decrement the read bytes counter */
            data_byte--;
        } 
    }
    
    I2C_Timeout = I2C_SHORT_TIMEOUT;
    /* wait until stop condition generate */ 
    while(I2C_CTL0(i2c_periph)&0x0200)
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }    
    
    /* enable acknowledge */
    i2c_ack_config(i2c_periph,I2C_ACK_ENABLE);

    i2c_ackpos_config(i2c_periph,I2C_ACKPOS_CURRENT);
	
	return RT_EOK;
}

int gd32_i2c_write(uint32_t i2c_periph,uint8_t slave_address,uint8_t* p_buffer,uint8_t write_address,uint16_t data_byte)
{
	uint32_t I2C_Timeout = I2C_SHORT_TIMEOUT;

    /* wait until I2C bus is idle */
    while((i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY)))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);
	I2C_Timeout = I2C_SHORT_TIMEOUT;
    while(!(i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND)))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }

	/* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, slave_address<<1, I2C_TRANSMITTER);
	/* wait until ADDSEND bit is set */
	I2C_Timeout = I2C_SHORT_TIMEOUT;
    while(!(i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND)))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	/* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

	/* wait until the transmit data buffer is empty */
    I2C_Timeout = I2C_SHORT_TIMEOUT;
    while(SET != i2c_flag_get( i2c_periph , I2C_FLAG_TBE))
    {
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	
    /* send the slave's internal address to write to : only one byte address */
    i2c_data_transmit(i2c_periph, write_address);
	
	I2C_Timeout = I2C_SHORT_TIMEOUT;
    /* wait until BTC bit is set */
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
    { 
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        } 
    }
	
    /* while there is data to be written */
    while(data_byte--){  
        i2c_data_transmit(i2c_periph, *p_buffer);
        
        /* point to the next byte to be written */
        p_buffer++;
        
        I2C_Timeout = I2C_SHORT_TIMEOUT;
        while (!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
        { 
            if((I2C_Timeout--) == 0) 
            { 
                Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
                return RT_ERROR; 
            } 
        }
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(i2c_periph);

    /* wait until the stop condition is finished */
	I2C_Timeout = I2C_SHORT_TIMEOUT;
	while(I2C_CTL0(i2c_periph)&0x0200)
	{
        if((I2C_Timeout--) == 0) 
        { 
            Resume_IIC(I2C_LONG_TIMEOUT,i2c_periph); 
            return RT_ERROR; 
        }
	}
	return RT_EOK;
}

int rt_hw_i2c_init(uint32_t i2c_periph)
{
	if(i2c_periph == I2C1)
	{
		#define I2C1_SPEED  100000//2000000时，fm17522发送时ComIrqReg寄存器中定时器时间用尽

		/* enable GPIOB clock */
		rcu_periph_clock_enable(RCU_GPIOB);

		/* connect PB10 to I2C1_SCL, PB11 to I2C1_SDA */
		gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);

		/* enable I2C clock */
		rcu_periph_clock_enable(RCU_I2C1);
		/* configure I2C clock */
		i2c_clock_config(I2C1,I2C1_SPEED,I2C_DTCY_2);

		i2c_enable(I2C1);
		/* enable acknowledge */
		i2c_ack_config(I2C1,I2C_ACK_ENABLE);   
	}

#ifdef RT_USING_I2C0
#define I2C0_SPEED  100000

    static struct gd32_i2c_bus gd32_i2c0;
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* connect PB6 to I2C0_SCL, PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    /* enable I2C clock */
    rcu_periph_clock_enable(RCU_I2C0);
    /* configure I2C clock */
    i2c_clock_config(I2C0,I2C0_SPEED,I2C_DTCY_2);
	rt_thread_mdelay(100);
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0,I2C_ACK_ENABLE);
	
    //IIC0的中断允许; Added by Kim
//    i2c_interrupt_enable(I2C0, I2C_INT_ERR);
//	i2c_interrupt_enable(I2C0, I2C_INT_EV);
//	i2c_interrupt_enable(I2C0, I2C_INT_BUF);
    //i2c_interrupt_enable(I2C0, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
	
    rt_memset((void *)&gd32_i2c0, 0, sizeof(struct gd32_i2c_bus));
    gd32_i2c0.parent.ops = &i2c_ops;
    gd32_i2c0.i2c_periph = I2C0;
    rt_i2c_bus_device_register(&gd32_i2c0.parent, "i2c0");
	
//	//初始化IIC0的信号量; Added by Kim
//    gd32_i2c_sem_init(&gd32_i2c0, "i2c0_dsem");
//    //保存总线变量; Added by Kim
//    gd32_i2c_save_bus_param(&gd32_i2c0);
	
#endif
    return 0;
}
