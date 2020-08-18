/*
 * File      : rtc.h
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
#ifndef __RTC_H__
#define __RTC_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <rtthread.h>
#include <board.h>

#define RTC_YEAR2000         2000            /*!< RTC Reference */
#define RTC_FCR_REFERENCE    40000           /*!< RTC Reference */

#define RTC_CLOCK_12         0                /*!< RTC 12 Hour */
#define RTC_CLOCK_24         1                /*!< RTC 24 Hour */

#define RTC_AM               1                /*!< RTC AM */
#define RTC_PM               2                /*!< RTC PM */

#define RTC_SUNDAY       ((uint32_t) 0x00000000) /*!< Day of week is sunday */
#define RTC_MONDAY       ((uint32_t) 0x00000001) /*!< Day of week is monday */
#define RTC_TUESDAY      ((uint32_t) 0x00000002) /*!< Day of week is tuesday */
#define RTC_WEDNESDAY    ((uint32_t) 0x00000003) /*!< Day of week is wednesday */
#define RTC_THURSDAY     ((uint32_t) 0x00000004) /*!< Day of week is thursday */
#define RTC_FRIDAY       ((uint32_t) 0x00000005) /*!< Day of week is friday */
#define RTC_SATURDAY     ((uint32_t) 0x00000006) /*!< Day of week is saturday */

/**
  * @brief  RTC define Time Data Struct
  */
typedef struct {
	uint32_t u32Second;        /*!<  Second value */
    uint32_t u32Minute;        /*!<  Minute value */	
    uint32_t u32Hour;          /*!<  Hour value */
    uint32_t u32Day;           /*!<  Day value */
    uint32_t u32Month;         /*!<  Month value */
    uint32_t u32Year;          /*!<  Year value */
    uint32_t u32DayOfWeek;     /*!<  Day of week value */
    uint32_t u32TimeScale;     /*!<  12-Hour, 24-Hour */
    uint32_t u32AmPm;          /*!<  Only Time Scale select 12-hr used */
} S_RTC_TIME_DATA_T;

extern void LocalTimeInit(void);
extern void LocalTimeSync(S_RTC_TIME_DATA_T* time );
extern void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt);
void RTC_TimerStart(uint32_t second);

#ifdef  __cplusplus
}
#endif

#endif

