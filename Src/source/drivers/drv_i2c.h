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
 */

#ifndef __DRV_I2C__
#define __DRV_I2C__

#include "gd32e10x.h"

#define I2C_SHORT_TIMEOUT 0x3fff
#define I2C_LONG_TIMEOUT 0x5f//0x5fff

int rt_hw_i2c_init(uint32_t i2c_periph);
int gd32_i2c_read(uint32_t i2c_periph,uint8_t slave_address,uint8_t* p_buffer,uint8_t read_address,uint8_t data_byte);
int gd32_i2c_write(uint32_t i2c_periph,uint8_t slave_address,uint8_t* p_buffer,uint8_t write_address,uint16_t data_byte);
#endif
