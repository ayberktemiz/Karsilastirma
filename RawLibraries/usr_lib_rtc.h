#ifndef __USR_RTC_LIB_H
#define __USR_RTC_LIB_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "gpio.h"
    #include "rtc.h"   
#endif

#define _DAY_SECOND_CONSTANT              86400

typedef struct S_TIMEDATE_TAG
{
    /**
     * 0 -> year [0-99] 0 means 2000
     * 1 -> month [1-12]
     * 2 -> dayofmonth [1-31]
     * 3 -> hour [0-23]
     * 4 -> minute [0-59]
     * 5 -> second [0-59]
     */
    uint8_t timeDateBuf[6];
    uint8_t dummyBuf[2];
}S_TIMEDATE;

typedef enum
{
    _year,
    _month,
    _date,
    _hour,
    _minute,
    _second,
    _endTimeDate
} ETimeDate;

uint32_t UL_RtcGetTs(void);
void UL_RtcSetTs(uint32_t f_ts);
uint32_t UL_RtcDeltaTime(uint32_t f_time);

#endif //__USR_RTC_H


