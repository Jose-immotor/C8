
#include "drv_rtc.h"
#include "date.h"
#include "common.h"
#include "datatime.h"

S_RTC_TIME_DATA_T sInitTime;

S_RTC_TIME_DATA_T* DateTime_MakeRtc(int year, int month, int day, int hour, int minute, int sec)
{
	static S_RTC_TIME_DATA_T g_dt = {0};
	g_dt.u32Year   = year;
	g_dt.u32Month  = month;
	g_dt.u32Day    = day ;
	
	g_dt.u32Hour   = hour;
	g_dt.u32Minute = minute;
	g_dt.u32Second = sec;
	return &g_dt;
}

//获取当前时间的Seconds
time_t DateTime_GetSeconds(const S_RTC_TIME_DATA_T* localDt)
{
	struct tm dt = {0};
	
	if(Null == localDt)
	{
		S_RTC_TIME_DATA_T g_dt = {0};
		RTC_GetDateAndTime(&g_dt);
		localDt = &g_dt;
	}
	
	DateTime_ToTm(localDt, &dt);
	
	return mktime(&dt);
}
struct tm* dt;
void DateTime_SecondsToRtc(time_t time, S_RTC_TIME_DATA_T* rtcTime)
{
	dt = localtime(&time);
	DateTime_FromTm(dt, rtcTime);
}

void DateTime_ToTm(const S_RTC_TIME_DATA_T* localDt, struct tm* dt)
{
	dt->tm_year = localDt->u32Year - 1900;
	dt->tm_mon  = localDt->u32Month - 1;
	dt->tm_mday  = localDt->u32Day;
	
	dt->tm_hour = localDt->u32Hour;
	dt->tm_min  = localDt->u32Minute;
	dt->tm_sec  = localDt->u32Second;
}

void DateTime_FromTm(const struct tm* dt, S_RTC_TIME_DATA_T* localDt)
{
	localDt->u32Year   = dt->tm_year + 1900;
	localDt->u32Month  = dt->tm_mon + 1;
	localDt->u32Day    = dt->tm_mday ;
	
	localDt->u32Hour   = dt->tm_hour;
	localDt->u32Minute = dt->tm_min ;
	localDt->u32Second = dt->tm_sec ;
}

void DateTime_Add(S_RTC_TIME_DATA_T* localDt, uint32 second)
{
	struct tm dt;
	struct tm* pDt;
	
	DateTime_ToTm(localDt, &dt);
	
	time_t time1 = mktime(&dt) + second;
	pDt = localtime(&time1);
	
	DateTime_FromTm(pDt, localDt);
	
//	DateTime_dump(localDt);
}

//两个时间相减，返回秒数
int DateTime_Sub(const S_RTC_TIME_DATA_T* srcDt, const S_RTC_TIME_DATA_T* dstDt)
{
	struct tm dt;
	
	if(Null == srcDt)
	{
		S_RTC_TIME_DATA_T g_dt = {0};
//		RTC_GetDateAndTime(&g_dt);
		srcDt = &g_dt;
	}
	
	DateTime_ToTm(srcDt, &dt);
	time_t time1 = mktime(&dt);
	
	DateTime_ToTm(dstDt, &dt);
	time_t time2 = mktime(&dt);
		
	return time1 - time2;
}

void DateTime_dump(S_RTC_TIME_DATA_T* dt)
{
	S_RTC_TIME_DATA_T sCurTime;
	if(dt == Null)
	{
	    /* Get the current time */
	    RTC_GetDateAndTime(&sCurTime);
		dt = &sCurTime;
	}
    Printf("DataTime:%d/%02d/%02d %02d:%02d:%02d\n",dt->u32Year,dt->u32Month,
           dt->u32Day,dt->u32Hour,dt->u32Minute,dt->u32Second);
}

char* DateTime_ToStr(uint32 seconds)
{
	static char buf[64] = {0};

	S_RTC_TIME_DATA_T sCurTime;
	struct tm* pDt = localtime(&seconds);
	DateTime_FromTm(pDt, &sCurTime);
	
    sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);

	return buf;
}

/**
 * @brief LocalTimeSync
 * @param[in] cnt 秒计数
 * @param[out] none
 * @param 
 * @return 
 * 
 * @attention 
 */
void LocalTimeSync(S_RTC_TIME_DATA_T* time )
{
	static struct rtc_time systime;
	static uint32_t timedata=0;
	
	/* Time Setting */
	Printf("DataTime sync:");
	DateTime_dump(time);
	
	sInitTime.u32Year       = time->u32Year;
	sInitTime.u32Month      = time->u32Month;
	sInitTime.u32Day        = time->u32Day;
	sInitTime.u32Hour       = time->u32Hour;
	sInitTime.u32Minute     = time->u32Minute;
	sInitTime.u32Second     = time->u32Second;
	
	
	memcpy(&systime,&sInitTime,sizeof(struct rtc_time));
	
	timedata = mktimev(&systime);
	
    /* change the current time */
    rtc_counter_set(timedata);
    rtc_lwoff_wait();	
}

/**
 *  @brief    Read current date/time from RTC setting
 *
 *  @param[out]    sPt \n
 *                     Specify the time property and current time. It includes: \n
 *                     u32Year: Year value                                      \n
 *                     u32Month: Month value                                    \n
 *                     u32Day: Day value                                        \n
 *                     u32DayOfWeek: Day of week                                \n
 *                     u32Hour: Hour value                                      \n
 *                     u32Minute: Minute value                                  \n
 *                     u32Second: Second value                                  \n
 *                     u32TimeScale: \ref RTC_CLOCK_12 / \ref RTC_CLOCK_24          \n
 *                     u8AmPm: \ref RTC_AM / \ref RTC_PM                            \n
 *
 *  @return   None
 *
 */
void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt)
{
	uint32_t timevar=0;
	static struct rtc_time systime;


	timevar = rtc_counter_get();
	
	to_tm(timevar, &systime);
	
	memcpy(&sInitTime,&systime,sizeof(struct rtc_time));
	
    sPt->u32TimeScale = sInitTime.u32TimeScale;    /* 12/24-hour */
    sPt->u32DayOfWeek = sInitTime.u32DayOfWeek;          /* Day of week */
    sPt->u32Year      = sInitTime.u32Year;
    sPt->u32Month     = sInitTime.u32Month;
    sPt->u32Day       = sInitTime.u32Day;	
	
    if (sInitTime.u32TimeScale == RTC_CLOCK_12)  /* Compute12/24 hour */
    {
		if(sInitTime.u32Hour >= 13)
		{
		    sPt->u32AmPm = RTC_PM;
			sPt->u32Hour = sInitTime.u32Hour - 12;
		}
		else
		{
		    sPt->u32AmPm = RTC_AM;
			sPt->u32Hour = sInitTime.u32Hour;
		}
		sPt->u32Minute   = sInitTime.u32Minute;
		sPt->u32Second   = sInitTime.u32Second;
    }
	else 
	{
        sPt->u32Hour     = sInitTime.u32Hour;
		sPt->u32Minute   = sInitTime.u32Minute;
		sPt->u32Second   = sInitTime.u32Second;
    }
}

//RTC定时唤醒
void RTC_TimerStart(uint32_t second)
{
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);	
	/* Enable RTC Clock */
	rcu_periph_clock_enable(RCU_RTC);
	/* Wait for RTC registers synchronization */
	rtc_register_sync_wait();

	rtc_lwoff_wait();
	rtc_alarm_config(rtc_counter_get()+second);
	/* Wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
		
	rtc_interrupt_enable(RTC_INT_ALARM);
	rtc_lwoff_wait();
}

/**
 * @brief LocalTimeInit
 * @param[in] none
 * @param[out] none
 * @param 
 * @return 
 * 
 * @attention 
 */
void LocalTimeInit(void)
{
    static struct rtc_time systime;
	/* Time Setting */
    sInitTime.u32Year       = 2020;
    sInitTime.u32Month      = 1;
    sInitTime.u32Day        = 1;
    sInitTime.u32Hour       = 12;
    sInitTime.u32Minute     = 0;
    sInitTime.u32Second     = 0;
    sInitTime.u32DayOfWeek  = RTC_WEDNESDAY;
    sInitTime.u32TimeScale  = RTC_CLOCK_24;
	sInitTime.u32AmPm       = RTC_AM;
	
	memcpy(&systime,&sInitTime,sizeof(struct rtc_time));

	rtc_configuration();	
    /* change the current time */
    rtc_counter_set(mktimev(&systime));
	/* Wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
}

void DateTime_Add_Test()
{
	S_RTC_TIME_DATA_T dataTime[] = 
	{
		 {2016, 11, 10, 0, 17, 30, 50, RTC_CLOCK_24}
		,{2016, 11, 10, 0, 23, 50, 50, RTC_CLOCK_24}
		,{2016, 11, 30, 0, 23, 50, 50, RTC_CLOCK_24}
		,{2016, 12, 31, 0, 23, 59, 50, RTC_CLOCK_24}
		,{2016, 02, 28, 0, 23, 59, 50, RTC_CLOCK_24}
		,{2016, 02, 29, 0, 23, 59, 50, RTC_CLOCK_24}
	};
//	S_RTC_TIME_DATA_T  dstDt;
	DateTime_dump(&dataTime[0]);

	DateTime_Add(&dataTime[0], 10);
	DateTime_Add(&dataTime[0], 60);
	DateTime_Add(&dataTime[0], 3600);
	DateTime_Add(&dataTime[0], 3600*24);

	
	Printf("DateTime_Add test passed.\n");
}

