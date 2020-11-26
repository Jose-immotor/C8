/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-5      SummerGift   first version
 */

#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include "gd32e10x.h"

#define FLASH_ADDR_UPDATE 0x0801fc80

int gd32_flash_read(uint32_t  addr, uint8_t *buf, uint16_t size);
int gd32_flash_write(uint32_t addr, uint8_t *buf, uint16_t size);
int gd32_flash_erase(uint32_t addr, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_FLASH_H__ */
