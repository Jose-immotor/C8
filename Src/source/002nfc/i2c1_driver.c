/*
 * File      : i2c_driver.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  nfc iic control
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-04     lane      the first version
 */

#include "i2c1_driver.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "fm175xx.h"

#define EEPROM_BLOCK0_ADDRESS    (0xA0>>1)
#define FM175XX_I2C_DEV_NAME "i2c2"



struct rt_i2c_bus_device *i2c_bus_device;


void iic_dev_init(void)
{
    i2c_bus_device = (struct rt_i2c_bus_device *)rt_device_find(FM175XX_I2C_DEV_NAME);
    //gl_fm17522_param.i2c_addr = FM17522_I2C_ADDR;
	
}

static rt_err_t write_reg_x(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;
    buf[0] = reg; //cmd
    //buf[1] = data[0];
    //buf[2] = data[1];
    msgs.addr = EEPROM_BLOCK0_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 1;
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;
    msgs.addr = EEPROM_BLOCK0_ADDRESS;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

rt_err_t i2c_read_fm17522_reg(rt_uint8_t reg_addr, rt_uint8_t *value)
{    
    struct rt_i2c_msg msgs[2];
    msgs[0].addr = Fm17522_get_slave_addr();
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg_addr;
    msgs[0].len = 1;
    msgs[1].addr = Fm17522_get_slave_addr();
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = value;
    msgs[1].len = 1;

    if (rt_i2c_transfer(i2c_bus_device, msgs, 2) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

rt_err_t i2c_read_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len)
{    
    struct rt_i2c_msg msgs[2];
    msgs[0].addr = Fm17522_get_slave_addr();
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &fifo_reg_addr;
    msgs[0].len = 1;
    msgs[1].addr = Fm17522_get_slave_addr();
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = fifo_buf;
    msgs[1].len = fifo_buf_len;

    if (rt_i2c_transfer(i2c_bus_device, msgs, 2) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

rt_err_t i2c_write_fm17522_reg(rt_uint8_t reg_addr, rt_uint8_t value)
{    
    struct rt_i2c_msg msgs[1];
    rt_uint8_t vl_buf[2];
    msgs[0].addr = Fm17522_get_slave_addr();
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = vl_buf;
    vl_buf[0] = reg_addr;
    vl_buf[1] = value;
    msgs[0].len = 2;

    if (rt_i2c_transfer(i2c_bus_device, msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}

rt_err_t i2c_write_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len)
{    
    struct rt_i2c_msg msgs[1];
    rt_uint8_t vl_buf[256];
    msgs[0].addr = Fm17522_get_slave_addr();
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = vl_buf;
    vl_buf[0] = fifo_reg_addr;
    if(fifo_buf_len > (256-1))
    {
        rt_memcpy(&vl_buf[1], fifo_buf, 255);
        msgs[0].len = 256;
    }
    else
    {
        rt_memcpy(&vl_buf[1], fifo_buf, fifo_buf_len);
        msgs[0].len = fifo_buf_len+1;
    }
    

    if (rt_i2c_transfer(i2c_bus_device, msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }

}
