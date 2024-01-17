#include "usr_lib_rtc.h"
#include <time.h>

#define _io  static
#define _iov static volatile

_io S_TIMEDATE RtcGetTimeDateProc(void);
_io void RtcSetTimeDateProc(S_TIMEDATE *f_pTimeDate);

uint32_t UL_RtcGetTs(void)
{
    S_TIMEDATE sTimeDate = RtcGetTimeDateProc();
    struct tm sTime;
    time_t ts;

    memset((void *)&sTime, 0, sizeof(struct tm));
    sTime.tm_year = (sTimeDate.timeDateBuf[_year] + 2000) - 1900;
    sTime.tm_mon = sTimeDate.timeDateBuf[_month] - 1;
    sTime.tm_mday = sTimeDate.timeDateBuf[_date];
    sTime.tm_hour = sTimeDate.timeDateBuf[_hour];
    sTime.tm_min = sTimeDate.timeDateBuf[_minute];
    sTime.tm_sec = sTimeDate.timeDateBuf[_second];
    ts = mktime(&sTime);
    return ts;
}

void UL_RtcSetTs(uint32_t f_ts)
{
    S_TIMEDATE sTimeDate;
    time_t rawTime = f_ts;
    struct tm sTime = *localtime(&rawTime);

    sTimeDate.timeDateBuf[_year] = (sTime.tm_year + 1900) - 2000;
    sTimeDate.timeDateBuf[_month] = sTime.tm_mon + 1;
    sTimeDate.timeDateBuf[_date] = sTime.tm_mday;
    sTimeDate.timeDateBuf[_hour] = sTime.tm_hour;
    sTimeDate.timeDateBuf[_minute] = sTime.tm_min;
    sTimeDate.timeDateBuf[_second] = sTime.tm_sec;

    RtcSetTimeDateProc(&sTimeDate);
}

uint32_t UL_RtcDeltaTime(uint32_t f_time)
{
    int ts = (int)UL_RtcGetTs();
    int temp = (int)f_time - ts;
    if(temp < 0)
        temp *=-1;
    return (uint32_t)temp;
}

_io S_TIMEDATE RtcGetTimeDateProc(void)
{
    S_TIMEDATE stimeDate;
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    stimeDate.timeDateBuf[_year] = sDate.Year;
    stimeDate.timeDateBuf[_month] = sDate.Month;
    stimeDate.timeDateBuf[_date] = sDate.Date;

    stimeDate.timeDateBuf[_hour] = sTime.Hours;
    stimeDate.timeDateBuf[_minute] = sTime.Minutes;
    stimeDate.timeDateBuf[_second] = sTime.Seconds;

    return stimeDate;
}

_io void RtcSetTimeDateProc(S_TIMEDATE *f_pTimeDate)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    sDate.Year = f_pTimeDate->timeDateBuf[_year];
    sDate.Month = f_pTimeDate->timeDateBuf[_month];
    sDate.Date = f_pTimeDate->timeDateBuf[_date];
    sDate.WeekDay = 1;

    sTime.Hours = f_pTimeDate->timeDateBuf[_hour];
    sTime.Minutes = f_pTimeDate->timeDateBuf[_minute];
    sTime.Seconds = f_pTimeDate->timeDateBuf[_second];
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
 
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}