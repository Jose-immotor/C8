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

#define FMC_PAGE_SIZE           ((uint16_t)0x400U)//1KB
#define FMC_WRITE_START_ADDR    ((uint32_t)0x08000000U)//127ҳ
#define FMC_WRITE_END_ADDR      ((uint32_t)0x0801ffffU)

int gd32_flash_read(uint32_t  addr, uint8_t *buf, uint16_t size);
int gd32_flash_write(uint32_t addr, uint8_t *buf, uint16_t size);
int gd32_flash_erase(uint32_t addr, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_FLASH_H__ */
