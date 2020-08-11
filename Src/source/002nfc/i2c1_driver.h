/*
 * File      : i2c_driver.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  nfc iic control
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-04     lane      the first version
 */
#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include <rtthread.h>

void iic_dev_init(void);
rt_err_t i2c_read_fm17522_reg(rt_uint8_t reg_addr, rt_uint8_t *value);
rt_err_t i2c_read_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len);
rt_err_t i2c_write_fm17522_reg(rt_uint8_t reg_addr, rt_uint8_t value);
rt_err_t i2c_write_fm17522_fifo(rt_uint8_t fifo_reg_addr, rt_uint8_t *fifo_buf, rt_uint8_t fifo_buf_len);


#endif //__I2C_DRIVER_H__
