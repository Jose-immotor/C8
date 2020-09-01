#ifndef __DATE_TIME__H_
#define __DATE_TIME__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "drv_rtc.h"
#include "time.h"

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


extern void LocalTimeSync(S_RTC_TIME_DATA_T* time );
extern void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt);
void RTC_TimerStart(uint32_t second);


char* DateTime_ToStr(uint32 seconds);
void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt);
void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt);

void DateTime_Add(S_RTC_TIME_DATA_T* dt, uint32 second);
S_RTC_TIME_DATA_T* DateTime_MakeRtc(int year, int month, int day, int hour, int minute, int sec);

//两个时间相减，返回秒数 2016 12 12 
int DateTime_Sub(const S_RTC_TIME_DATA_T* srcDt, const S_RTC_TIME_DATA_T* dstDt);
time_t DateTime_GetSeconds(const S_RTC_TIME_DATA_T* localDt);
void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime);


void LocalTimeInit(void);

#ifdef __cplusplus
}
#endif

#endif


